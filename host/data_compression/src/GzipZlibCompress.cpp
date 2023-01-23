#include "GzipZlibCompress.hpp"
#include <sstream>

void GzipZlibCompressWorkshop::processContinuous(){
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
	std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(CHUNK_SIZE_IN_BYTE);
	std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(CHUNK_SIZE_IN_BYTE*MCR);
	std::vector<uint32_t, aligned_allocator<uint32_t>> compressedSizeBufferHost(1);

	// Device buffers
	BufferPointer inputBuffer(Application::getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(uint8_t)*inputBufferHost.size(), inputBufferHost.data());
	BufferPointer outputBuffer(Application::getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint8_t)*outputBufferHost.size(), outputBufferHost.data());
	BufferPointer compressedSizeBuffer(Application::getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint32_t), compressedSizeBufferHost.data());

	KernelPointer compressKernelMM(Application::getProgram<Lib::GZIP_ZLIB_COMPRESSION>(), "xilGzipZlibCompressMM:{xilGzipZlibCompressMM_1}");
	compressKernelMM->setArg(0, *inputBuffer);
	compressKernelMM->setArg(1, *outputBuffer);
	compressKernelMM->setArg(2, *compressedSizeBuffer);
	compressKernelMM->setArg(3, *checksumDataBuffer);
	compressKernelMM->setArg(5, checksum_type);

	CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

	bool last;

	// compress the input block by block
	do{
		uint32_t chunkSize=inputStream.pop(inputBufferHost.data(), CHUNK_SIZE_IN_BYTE, last);
		std::cout<<"inner reads a "<<chunkSize<<" Bytes block"<<std::endl;

		// set the kernel input size accordingly
		compressKernelMM->setArg(4, chunkSize);

		// transfer host side input buffer to kernel
		OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*inputBuffer, *checksumDataBuffer}, 0));
		queue->finish();

		// execute the kernel
		OCL_CHECK(err, err=queue->enqueueTask(*compressKernelMM));
		queue->finish();

		// Migrate the block sizes back
		OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*compressedSizeBuffer}, CL_MIGRATE_MEM_OBJECT_HOST));
		queue->finish();

		// transfer the data from device to host
		OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*outputBuffer, *checksumDataBuffer}, CL_MIGRATE_MEM_OBJECT_HOST));
		queue->finish();

		std::cout<<"inner writes a "<<compressedSizeBufferHost[0]<<" Bytes block"<<std::endl;
		outputStream.push(outputBufferHost.data(), compressedSizeBufferHost[0], false);

		if(checksum_type) checksumDataBufferHost.data()[0] = ~checksumDataBufferHost.data()[0];
	}while(!last);

	// Add last block header
	long int last_block=0xffff000001;
	outputStream.push(&last_block, 5, true);

	checksum=checksumDataBufferHost.data()[0];
}

void GzipZlibCompressWorkshop::processOverlapped(){
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
	std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(CHUNK_SIZE_IN_BYTE);
	std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(CHUNK_SIZE_IN_BYTE*MCR);
	std::vector<uint32_t, aligned_allocator<uint32_t>> compressedSizeBufferHost(1);

	KernelPointer compressKernelMM(Application::getProgram<Lib::GZIP_ZLIB_COMPRESSION>(), "xilGzipZlibCompressMM:{xilGzipZlibCompressMM_1}");
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

	uint64_t curItem=0;

	bool last;

	// compress the input block by block
	do{
		OCL_CHECK(err, inputEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, migrationH2DEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, kernelExeutionEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, migrationD2HEvents.emplace_back(Application::getContext(), &err))
		OCL_CHECK(err, outputEvents.emplace_back(Application::getContext(), &err))

		migrationH2DEventsDep.emplace_back();
		kernelExeutionEventsDep.emplace_back();
		migrationD2HEventsDep.emplace_back();

		if(curItem>0) migrationH2DEvents[curItem-1].wait();

		uint32_t chunkSize=inputStream.pop(inputBufferHost.data(), CHUNK_SIZE_IN_BYTE, last);
		std::cout<<"inner reads a "<<chunkSize<<" Bytes block"<<std::endl;

		OCL_CHECK(err, err=inputEvents[curItem].setStatus(CL_COMPLETE))

		// Device buffers
		BufferPointer inputBuffer(Application::getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(uint8_t)*inputBufferHost.size(), inputBufferHost.data());
		BufferPointer outputBuffer(Application::getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint8_t)*outputBufferHost.size(), outputBufferHost.data());
		BufferPointer compressedSizeBuffer(Application::getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint32_t), compressedSizeBufferHost.data());

		// transfer host side input buffer to kernel
		migrationH2DEventsDep[curItem].push_back(inputEvents[curItem]);
		if(curItem>0) migrationH2DEventsDep[curItem].push_back(kernelExeutionEvents[curItem-1]);
		OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*inputBuffer, *checksumDataBuffer}, 0, &migrationH2DEventsDep[curItem], &migrationH2DEvents[curItem]));
		// queue->finish();

		// set the kernel input size accordingly
		compressKernelMM->setArg(0, *inputBuffer);
		compressKernelMM->setArg(1, *outputBuffer);
		compressKernelMM->setArg(2, *compressedSizeBuffer);
		compressKernelMM->setArg(3, *checksumDataBuffer);
		compressKernelMM->setArg(4, chunkSize);
		compressKernelMM->setArg(5, checksum_type);

		// execute the kernel
		kernelExeutionEventsDep[curItem].push_back(migrationH2DEvents[curItem]);
		if(curItem>0) kernelExeutionEventsDep[curItem].push_back(migrationD2HEvents[curItem-1]);
		OCL_CHECK(err, err=queue->enqueueTask(*compressKernelMM, &kernelExeutionEventsDep[curItem], &kernelExeutionEvents[curItem]));
		// queue->finish();

		// Migrate the block sizes back
		migrationD2HEventsDep[curItem].push_back(kernelExeutionEvents[curItem]);
		if(curItem>0) migrationD2HEventsDep[curItem].push_back(outputEvents[curItem-1]);
		OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*compressedSizeBuffer, *outputBuffer, *checksumDataBuffer}, CL_MIGRATE_MEM_OBJECT_HOST, &migrationD2HEventsDep[curItem], &migrationD2HEvents[curItem]));
		// queue->finish();

		outputThreads.emplace_back([this, curItem, &compressedSizeBufferHost, &outputBufferHost, &migrationD2HEvents, &outputEvents, checksum_type, &checksumDataBufferHost]{
			migrationD2HEvents[curItem].wait();
			std::cout<<"inner writes a "<<compressedSizeBufferHost[0]<<" Bytes block"<<std::endl;
			outputStream.push(outputBufferHost.data(), compressedSizeBufferHost[0], false);
			if(checksum_type) checksumDataBufferHost.data()[0] = ~checksumDataBufferHost.data()[0];
			OCL_CHECK(err, cl_int err=outputEvents[curItem].setStatus(CL_COMPLETE))
		});

		curItem++;
	}while(!last);

	checksum=checksumDataBufferHost.data()[0];

	for(auto &t : outputThreads) t.join();
}

GzipZlibCompressWorkshop::GzipZlibCompressWorkshop(bool isZlib, bool overlapped) : 
	Workshop("GzipZlibInputStream", 4, "GzipZlibOutputStream", 4),
	isZlib(isZlib),
	processThread(overlapped ? &GzipZlibCompressWorkshop::processOverlapped : &GzipZlibCompressWorkshop::processContinuous, this){}

void GzipZlibCompressWorkshop::wait(){
	processThread.join();

	// Add last block header
	long int last_block=0xffff000001;
	outputStream.push(&last_block, 5, true);
}

uint32_t GzipZlibCompressWorkshop::getChecksum(){
	return checksum;
}

ByteStream& GzipZlibCompressWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& GzipZlibCompressWorkshop::getOutputStream(){
	return outputStream;
}