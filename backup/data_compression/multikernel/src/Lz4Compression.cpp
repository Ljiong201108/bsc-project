#include "Lz4Compression.hpp"
#include <sstream>

void Lz4CompressionKernelExecutor::process(){
	cl_int err;

	CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
	std::stringstream lz4CompressKernelName;
	lz4CompressKernelName<<"xilLz4CompressMM:{xilLz4CompressMM_"<<idx<<"}";
	std::cout<<"kernel name="<<lz4CompressKernelName.str()<<std::endl;
	KernelPointer lz4CompressKernel(Application::getProgram<Lib::LZ4_COMPRESSION>(), lz4CompressKernelName.str());

	std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(Lz4CompressionWorkshop::HOST_BUFFER_SIZE);
	std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(Lz4CompressionWorkshop::HOST_BUFFER_SIZE);
	std::vector<uint32_t, aligned_allocator<uint32_t>> blockSizeBufferHost(Lz4CompressionWorkshop::MAX_NUMBER_BLOCKS);
	std::vector<uint32_t, aligned_allocator<uint32_t>> compressSizeBufferHost(Lz4CompressionWorkshop::MAX_NUMBER_BLOCKS);

	// Device buffer allocation
	BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, inBufferHost.size(), inBufferHost.data());
	BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, outBufferHost.size(), outBufferHost.data());
	BufferPointer compressdSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t)*compressSizeBufferHost.size(), compressSizeBufferHost.data());
	BufferPointer buffer_block_size(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t)*blockSizeBufferHost.size(), blockSizeBufferHost.data());

	lz4CompressKernel->setArg(0, *(inputBuffer));
	lz4CompressKernel->setArg(1, *(outputBuffer));
	lz4CompressKernel->setArg(2, *(compressdSizeBuffer));
	lz4CompressKernel->setArg(3, *(buffer_block_size));
	lz4CompressKernel->setArg(4, (uint32_t)Lz4CompressionWorkshop::BLOCK_SIZE_IN_KB);

	std::vector<cl::UserEvent> inputEvents;
	std::vector<cl::UserEvent> migrationH2DEvents;
	std::vector<cl::UserEvent> kernelExeutionEvents;
	std::vector<cl::UserEvent> migrationD2HEvents;
	std::vector<cl::UserEvent> outputEvents;

	std::vector<std::vector<cl::Event>> migrationH2DEventsDep;
	std::vector<std::vector<cl::Event>> kernelExeutionEventsDep;
	std::vector<std::vector<cl::Event>> migrationD2HEventsDep;

	std::vector<std::thread> outputThreads;

	uint64_t curItem=0;

	while(true){
		if(curItem>0) migrationH2DEvents[curItem-1].wait();

		Lz4CompressionItem item=inputStream.pop();
		uint32_t chunkSize=item.size;
		uint64_t itemIdx=item.idx;

		if(itemIdx==(uint64_t)-1) break;

		memcpy(inBufferHost.data(), item.payload, chunkSize);
		delete[] item.payload;

		OCL_CHECK(err, inputEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, migrationH2DEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, kernelExeutionEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, migrationD2HEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, outputEvents.emplace_back(Application::getContext(), &err))

		migrationH2DEventsDep.emplace_back();
		kernelExeutionEventsDep.emplace_back();
		migrationD2HEventsDep.emplace_back();

		OCL_CHECK(err, cl_int err=inputEvents[curItem].setStatus(CL_COMPLETE))
		std::cout<<"kernel executor "<<idx<<" get input ["<<chunkSize<<" Bytes]"<<std::endl;
		
		uint32_t numBlocks=(chunkSize-1)/Lz4CompressionWorkshop::BLOCK_SIZE_IN_BYTE+1;
		
		uint32_t bIdx=0;
		for (uint32_t bs=0;bs<chunkSize;bs+=Lz4CompressionWorkshop::BLOCK_SIZE_IN_BYTE){
			uint32_t blockSize=Lz4CompressionWorkshop::BLOCK_SIZE_IN_BYTE;
			if(bs+blockSize>chunkSize) blockSize=chunkSize-bs;
			blockSizeBufferHost.data()[bIdx++] = blockSize;
		}

		migrationH2DEventsDep[curItem].push_back(inputEvents[curItem]);
		if(curItem>0) migrationH2DEventsDep[curItem].push_back(kernelExeutionEvents[curItem-1]);
		OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*inputBuffer, *buffer_block_size}, 0, &migrationH2DEventsDep[curItem], &migrationH2DEvents[curItem]))

		lz4CompressKernel->setArg(5, chunkSize);
		kernelExeutionEventsDep[curItem].push_back(migrationH2DEvents[curItem]);
		if(curItem>0) kernelExeutionEventsDep[curItem].push_back(migrationD2HEvents[curItem-1]);
		OCL_CHECK(err, err=queue->enqueueTask(*lz4CompressKernel, &kernelExeutionEventsDep[curItem], &kernelExeutionEvents[curItem]))
		
		migrationD2HEventsDep[curItem].push_back(kernelExeutionEvents[curItem]);
		if(curItem>0) migrationD2HEventsDep[curItem].push_back(outputEvents[curItem-1]);
		OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*outputBuffer, *compressdSizeBuffer}, CL_MIGRATE_MEM_OBJECT_HOST, &migrationD2HEventsDep[curItem], &migrationD2HEvents[curItem]))

		outputThreads.emplace_back([this, curItem, itemIdx, &migrationD2HEvents, numBlocks, chunkSize, &compressSizeBufferHost, &outBufferHost, &inBufferHost, &outputEvents]{
			migrationD2HEvents[curItem].wait();
			std::cout<<"going to push item "<<itemIdx<<std::endl;
			std::cout<<"It's No. "<<curItem<<" of this executor with "<<numBlocks<<" blocks"<<std::endl;
			std::unique_lock<std::mutex> lck(workshop.mtx);
			workshop.cv.wait(lck, [this, itemIdx] {return workshop.outputIdx==itemIdx;});

			uint32_t idx = 0;
			for (uint32_t bIdx=0; bIdx<numBlocks;bIdx++, idx+=Lz4CompressionWorkshop::BLOCK_SIZE_IN_BYTE) {
				uint32_t blockSize=Lz4CompressionWorkshop::BLOCK_SIZE_IN_BYTE;
				if (idx+blockSize>chunkSize) blockSize=chunkSize-idx;
				uint32_t compressed_size=compressSizeBufferHost.data()[bIdx];
				assert(compressed_size!=0);

				uint32_t percCal=chunkSize*10;
				percCal=percCal/blockSize;

				if (compressed_size<blockSize && percCal>=10) {
					outputStream.push(&compressed_size, 4, false);

					outputStream.push(&(outBufferHost.data()[bIdx*Lz4CompressionWorkshop::BLOCK_SIZE_IN_BYTE]), compressed_size, false);
					// std::cout<<bIdx<<" compressed block size: "<<compressed_size<<std::endl;
				} else {
					uint8_t temp = 0;
					temp=blockSize;
					outputStream.push(&temp, 1, false);
					temp=blockSize >> 8;
					outputStream.push(&temp, 1, false);
					temp=0;
					outputStream.push(&temp, 1, false);
					temp=128;
					outputStream.push(&temp, 1, false);

					outputStream.push(inBufferHost.data()+idx, blockSize, false);
					// std::cout<<bIdx<<" compressed block size: "<<blockSize<<std::endl;
				}
			}

			OCL_CHECK(err, cl_int err=outputEvents[curItem].setStatus(CL_COMPLETE))
			workshop.outputIdx++;
			workshop.cv.notify_all();
			std::cout<<"Finished pushing item "<<itemIdx<<std::endl;
		});

		curItem++;
	}

	for(auto &t : outputThreads) t.join();
	for(auto &e : inputEvents) e.wait();
	for(auto &e : migrationH2DEvents) e.wait();
	for(auto &e : kernelExeutionEvents) e.wait();
	for(auto &e : migrationD2HEvents) e.wait();
	for(auto &e : outputEvents) e.wait();

	std::cout<<"End of kernel executor "<<idx<<std::endl;
}

Lz4CompressionKernelExecutor::Lz4CompressionKernelExecutor(
	int idx,
	Stream<Lz4CompressionItem> &inputStream, 
	ByteStream &outputStream,  Lz4CompressionWorkshop &workshop) 
	: KernelExecutor<Lz4CompressionItem>(idx, inputStream, outputStream), workshop(workshop){}

void Lz4CompressionStructureAnalyzer::process(){
	bool last;
	uint64_t idx=0;

	// compress the input block by block
	do{
		uint8_t *payload=new uint8_t[Lz4CompressionWorkshop::HOST_BUFFER_SIZE];
		uint32_t chunkSize=inputStream.pop(payload, Lz4CompressionWorkshop::HOST_BUFFER_SIZE, last);
		std::cout<<"inner reads a "<<chunkSize<<" Bytes block"<<std::endl;

		Lz4CompressionItem item;
		item.idx=idx++;
		item.size=chunkSize;
		item.payload=payload;
		outputStream.push(item);
	}while(!last);

	std::cout<<"End of structure analyzer"<<std::endl;
}

Lz4CompressionStructureAnalyzer::Lz4CompressionStructureAnalyzer(
		ByteStream &inputStream,
		Stream<Lz4CompressionItem> &outputStream) : 
		StructureAnalyzer<Lz4CompressionItem>(inputStream, outputStream){}

Lz4CompressionWorkshop::Lz4CompressionWorkshop() : 
	Workshop<Lz4CompressionItem>("Lz4InputStream", 4, "Lz4BridgeStream", 4, "Lz4OutputStream", 4), 
	kernelExecutor1(1, bridgeStream, outputStream, *this),
	kernelExecutor2(2, bridgeStream, outputStream, *this),
	structureAnalyzer(inputStream, bridgeStream),
	structureAnalyzerThread(&Lz4CompressionStructureAnalyzer::process, &structureAnalyzer),
	kernelExecutorThread1(&Lz4CompressionKernelExecutor::process, &kernelExecutor1),
	kernelExecutorThread2(&Lz4CompressionKernelExecutor::process, &kernelExecutor2){
}

void Lz4CompressionWorkshop::wait(){
	structureAnalyzerThread.join();

	std::cout<<"structureAnalyzerThread is joined"<<std::endl;

	Lz4CompressionItem item;
	item.idx=(uint64_t)-1;
	item.size=0;
	item.payload=nullptr;
	bridgeStream.push(item);
	bridgeStream.push(item);

	kernelExecutorThread1.join();
	kernelExecutorThread2.join();

	uint8_t tmp=0;
	outputStream.push(&tmp, 1, true);

	std::cout<<"kernelExecutorThread is joined"<<std::endl;
}

ByteStream& Lz4CompressionWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& Lz4CompressionWorkshop::getOutputStream(){
	return outputStream;
}