#include "Lz4Compress.hpp"

void Lz4CompressWorkshop::process(){
	Timer::startAnaTimer();
	uint32_t hostBufferSize = HOST_BUFFER_SIZE;
    uint32_t maxNumBlocks = (hostBufferSize) / (BLOCK_SIZE_IN_KB * 1024);
    std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(hostBufferSize);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(hostBufferSize);
    std::vector<uint32_t, aligned_allocator<uint32_t>> blockSizeBufferHost(maxNumBlocks);
    std::vector<uint32_t, aligned_allocator<uint32_t>> compressSizeBufferHost(maxNumBlocks);

    uint32_t block_size_in_kb = BLOCK_SIZE_IN_KB;
    uint32_t block_size_in_bytes = block_size_in_kb * 1024;

	Timer::startFPGAInitTimer();
    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
    KernelPointer lz4CompressKernel(Application::getProgram<Lib::LZ4>(), "xilLz4CompressMM:{xilLz4CompressMM_1}");
    bool last;
	Timer::endFPGAInitTimer();

    do{
        uint32_t chunkSize=inputStream.pop(inBufferHost.data(), hostBufferSize, last);
        // std::cout<<"get input ["<<chunkSize<<" Bytes]"<<std::endl;
        // hexdump(inBufferHost.data(), chunkSize);
        
        uint32_t numBlocks = (chunkSize - 1) / block_size_in_bytes + 1;
        
        uint32_t bIdx = 0;
        for (uint32_t bs = 0; bs < chunkSize; bs += block_size_in_bytes) {
            uint32_t blockSize = block_size_in_bytes;
            if (bs + blockSize > chunkSize) {
                blockSize = chunkSize - bs;
            }
            blockSizeBufferHost.data()[bIdx++] = blockSize;
        }
        // Calculate chunks size in bytes for device buffer creation
        uint32_t bufferSize = ((chunkSize - 1) / BLOCK_SIZE_IN_KB + 1) * BLOCK_SIZE_IN_KB;

		Timer::startFPGAInitTimer();
        // Device buffer allocation
        BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, bufferSize, inBufferHost.data());
        BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, bufferSize, outBufferHost.data());
        BufferPointer compressdSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t) * numBlocks, compressSizeBufferHost.data());
        BufferPointer buffer_block_size(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t) * numBlocks, blockSizeBufferHost.data());

        // Set kernel arguments
        uint32_t narg = 0;
        lz4CompressKernel->setArg(narg++, *(inputBuffer));
        lz4CompressKernel->setArg(narg++, *(outputBuffer));
        lz4CompressKernel->setArg(narg++, *(compressdSizeBuffer));
        lz4CompressKernel->setArg(narg++, *(buffer_block_size));
        lz4CompressKernel->setArg(narg++, block_size_in_kb);
        lz4CompressKernel->setArg(narg++, chunkSize);

        queue->enqueueMigrateMemObjects({*inputBuffer, *buffer_block_size}, 0 /* 0 means from host*/);
        queue->finish();

        queue->enqueueTask(*lz4CompressKernel);
        queue->finish();
        
        queue->enqueueMigrateMemObjects({*outputBuffer, *compressdSizeBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
        queue->finish();
		Timer::endFPGAInitTimer();

        uint32_t idx = 0;
        for (uint32_t bIdx = 0; bIdx < numBlocks; bIdx++, idx += block_size_in_bytes) {
            uint32_t blockSize = block_size_in_bytes;
            if (idx + blockSize > chunkSize) {
                blockSize = chunkSize - idx;
            }
            uint32_t compressed_size = compressSizeBufferHost.data()[bIdx];
            assert(compressed_size != 0);

            uint32_t percCal = chunkSize * 10;
            percCal = percCal / blockSize;

            if (compressed_size < blockSize && percCal >= 10) {
                outputStream.push(&compressed_size, 4, false);

                if(bIdx<numBlocks-1) outputStream.push(&(outBufferHost.data()[bIdx * block_size_in_bytes]), compressed_size, false);
                else outputStream.push(&(outBufferHost.data()[bIdx * block_size_in_bytes]), compressed_size, last);
                // std::cout<<"compressed block size: "<<compressed_size<<std::endl;
                // hexdump(&(outBufferHost.data()[bIdx * block_size_in_bytes]), compressed_size);
            } else {
                uint8_t temp[4];
                temp[0] = blockSize;
                temp[1] = blockSize >> 8;
				temp[2]=0;
				temp[3]=128;
                outputStream.push(&temp, 4, false);

                if(bIdx<numBlocks-1) outputStream.push(inBufferHost.data()+idx, blockSize, false);
                else outputStream.push(inBufferHost.data()+idx, blockSize, last);
                // std::cout<<"compressed block size: "<<blockSize<<std::endl;
                // hexdump(inBufferHost.data()+idx, blockSize);
            }
        }
    }while(!last);
	Timer::endAnaTimer();
}

Lz4CompressWorkshop::Lz4CompressWorkshop() : 
	Workshop("Lz4InputStream", 1<<30, "Lz4OutputStream", 1<<30),
	processThread(&Lz4CompressWorkshop::process, this){}

void Lz4CompressWorkshop::wait(){
	processThread.join();
}

ByteStream& Lz4CompressWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& Lz4CompressWorkshop::getOutputStream(){
	return outputStream;
}