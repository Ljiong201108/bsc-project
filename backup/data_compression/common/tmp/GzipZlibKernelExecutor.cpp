#include "GzipZlibWorkshop.hpp"
#include <sstream>

void GzipZlibCompressionKernelExecutor::process(){
	cl_int err;

	std::vector<uint32_t, aligned_allocator<uint32_t>> checksumDataBufferHost(1);
	BufferPointer checksumDataBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), checksumDataBufferHost.data());
	
	bool checksum_type;

	if(isZlib){
		checksumDataBufferHost.data()[0] = 1;
		checksum_type = false;
	} else {
		checksumDataBufferHost.data()[0] = ~0;
		checksum_type = true;
	}

	// Host Buffers
	std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(GzipZlibCompressionWorkshop::CHUNK_SIZE_IN_BYTE);
	std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(GzipZlibCompressionWorkshop::CHUNK_SIZE_IN_BYTE*GzipZlibCompressionWorkshop::MCR);
	std::vector<uint32_t, aligned_allocator<uint32_t>> compressedSizeBufferHost(1);

	// Device buffers
	BufferPointer inputBuffer(Application::getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(uint8_t)*inputBufferHost.size(), inputBufferHost.data());
	BufferPointer outputBuffer(Application::getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint8_t)*outputBufferHost.size(), outputBufferHost.data());
	BufferPointer compressedSizeBuffer(Application::getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint32_t), compressedSizeBufferHost.data());

	std::stringstream kernelNameStream;
	kernelNameStream<<"xilGzipZlibCompressMM:{xilGzipZlibCompressMM_"<<idx<<"}";
	KernelPointer compressKernelMM(Application::getProgram<Lib::GZIP_ZLIB>(), kernelNameStream.str());
	compressKernelMM->setArg(0, *inputBuffer);
	compressKernelMM->setArg(1, *outputBuffer);
	compressKernelMM->setArg(2, *compressedSizeBuffer);
	compressKernelMM->setArg(3, *checksumDataBuffer);
	compressKernelMM->setArg(5, checksum_type);

	CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

	std::vector<cl::UserEvent> inputEvents;
	std::vector<cl::UserEvent> migrationH2DEvents;
	std::vector<cl::UserEvent> kernelExeutionEvents;
	std::vector<cl::UserEvent> migrationD2HEvents;
	std::vector<cl::UserEvent> outputEvents;

	std::vector<std::vector<cl::Event>> migrationH2DEventsDep;
	std::vector<std::vector<cl::Event>> kernelExeutionEventsDep;
	std::vector<std::vector<cl::Event>> migrationD2HEventsDep;

	std::vector<std::thread> outputThreads;

	//-----------------------------------------------------------------------------------------------------------

	uint64_t curItem=0;

	while(true){
		if(curItem>0) migrationH2DEvents[curItem-1].wait();
		GzipZlibCompressionItem item=inputQueue.pop();
		uint64_t itemIdx=item.idx;
		uint32_t itemPayloadSize=item.size;
		uint8_t *itemPayload=item.payload;

		std::cout<<"kernel executor["<<idx<<"] get an item"<<std::endl;
		std::cout<<"It's No. "<<curItem<<" of this executor"<<std::endl;
		std::cout<<"item: idx="<<itemIdx<<" size="<<itemPayloadSize<<std::endl;

		if(itemIdx==(uint64_t)-1) break;

		OCL_CHECK(err, inputEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, migrationH2DEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, kernelExeutionEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, migrationD2HEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, outputEvents.emplace_back(Application::getContext(), &err))

		migrationH2DEventsDep.emplace_back();
		kernelExeutionEventsDep.emplace_back();
		migrationD2HEventsDep.emplace_back();

		// inputThreads.emplace_back(&GzipKernelExecutor::executeInput, this);
		std::memcpy(inputBufferHost.data(), itemPayload, itemPayloadSize);
		delete[] itemPayload;
		OCL_CHECK(err, cl_int err=inputEvents[curItem].setStatus(CL_COMPLETE))

		// executeMigrationH2D();
		migrationH2DEventsDep[curItem].push_back(inputEvents[curItem]);
		if(curItem>0) migrationH2DEventsDep[curItem].push_back(kernelExeutionEvents[curItem-1]);
		OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*inputBuffer, *checksumDataBuffer}, 0, &migrationH2DEventsDep[curItem], &migrationH2DEvents[curItem]))

		// executeKernel();
		compressKernelMM->setArg(4, itemPayloadSize);
		kernelExeutionEventsDep[curItem].push_back(migrationH2DEvents[curItem]);
		if(curItem>0) kernelExeutionEventsDep[curItem].push_back(migrationD2HEvents[curItem-1]);
		OCL_CHECK(err, err=queue->enqueueTask(*compressKernelMM, &kernelExeutionEventsDep[curItem], &kernelExeutionEvents[curItem]));

		// executeMigrationD2H();
		migrationD2HEventsDep[curItem].push_back(kernelExeutionEvents[curItem]);
		if(curItem>0) migrationD2HEventsDep[curItem].push_back(outputEvents[curItem-1]);
		OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*compressedSizeBuffer, *outputBuffer, *checksumDataBuffer}, CL_MIGRATE_MEM_OBJECT_HOST, &migrationD2HEventsDep[curItem], &migrationD2HEvents[curItem]));

		// outputThreads.emplace_back(&GzipKernelExecutor::executeOutput, this);
		outputThreads.emplace_back([this, itemIdx, curItem, &outputEvents, &outputBufferHost, &compressedSizeBufferHost, &migrationD2HEvents]{
			migrationD2HEvents[curItem].wait();
			std::cout<<"going to push item "<<itemIdx<<" with size "<<compressedSizeBufferHost[0]<<std::endl;
			std::cout<<"It's No. "<<curItem<<" of this executor!"<<std::endl;
			std::unique_lock<std::mutex> lck(workshop.mtx);
			workshop.cv.wait(lck, [this, itemIdx] {return workshop.outputIdx==itemIdx;});

			outputQueue.push(outputBufferHost.data(), compressedSizeBufferHost[0], false);
			OCL_CHECK(err, cl_int err=outputEvents[curItem].setStatus(CL_COMPLETE))
			workshop.outputIdx++;
			workshop.cv.notify_all();
			std::cout<<"Finished pushing item "<<itemIdx<<std::endl;
		});

		curItem++;
	}

	// for(auto &t : inputThreads) t.join();
	for(auto &t : outputThreads) t.join();
	for(auto &e : inputEvents) e.wait();
	for(auto &e : migrationH2DEvents) e.wait();
	for(auto &e : kernelExeutionEvents) e.wait();
	for(auto &e : migrationD2HEvents) e.wait();
	for(auto &e : outputEvents) e.wait();

	checksum=checksumDataBufferHost[0];

	std::cout<<"End of kernel executor "<<idx<<std::endl;
}

GzipZlibCompressionKernelExecutor::GzipZlibCompressionKernelExecutor(
	int idx,
	bool isZlib,
	Stream<GzipZlibCompressionItem> &inputQueue, 
	ByteQueue &outputQueue,  GzipZlibCompressionWorkshop &workshop) 
	: KernelExecutor<GzipZlibCompressionItem>(idx, inputQueue, outputQueue), workshop(workshop), isZlib(isZlib){

}