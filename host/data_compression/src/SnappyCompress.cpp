#include "SnappyCompress.hpp"

void SnappyCompressWorkshop::process(){
	KernelPointer compressKernel(Application::getProgram<Lib::SNAPPY>(), "xilSnappyCompressMM:{xilSnappyCompressMM_1}");
    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(HOST_BUFFER_SIZE);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(HOST_BUFFER_SIZE);
    std::vector<uint32_t, aligned_allocator<uint32_t>> compressedSizeBufferHost(MAX_NUMBER_BLOCKS);
    std::vector<uint32_t, aligned_allocator<uint32_t>> decompressedSizeBufferHost(MAX_NUMBER_BLOCKS);

    uint32_t block_size_in_kb=BLOCK_SIZE_IN_KB;
    uint32_t block_size_in_bytes=block_size_in_kb*1024;
    bool last;

    do{
        uint32_t chunkSize=inputStream.pop(inputBufferHost.data(), HOST_BUFFER_SIZE, last);
        std::cout<<"inner reads a "<<chunkSize<<" Bytes block"<<std::endl;
        
        uint32_t numBlocks=(chunkSize-1)/block_size_in_bytes+1;
        
        uint32_t bIdx=0;
        for (uint32_t bs=0;bs<chunkSize;bs+=block_size_in_bytes) {
            uint32_t blockSize=block_size_in_bytes;
            if (bs+blockSize>chunkSize) blockSize=chunkSize-bs;
            decompressedSizeBufferHost[bIdx++]=blockSize;
        }

        // Calculate chunks size in bytes for device buffer creation
        uint32_t bufferSize=((chunkSize-1)/BLOCK_SIZE_IN_KB+1)*BLOCK_SIZE_IN_KB;

        BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, bufferSize, inputBufferHost.data());
        BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, bufferSize, outputBufferHost.data());
        BufferPointer compressedSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t)*numBlocks, compressedSizeBufferHost.data());
        BufferPointer decompressSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t)*numBlocks, decompressedSizeBufferHost.data());

        compressKernel->setArg(0, *inputBuffer);
        compressKernel->setArg(1, *outputBuffer);
        compressKernel->setArg(2, *compressedSizeBuffer);
        compressKernel->setArg(3, *decompressSizeBuffer);
        compressKernel->setArg(4, block_size_in_kb);
        compressKernel->setArg(5, chunkSize);

        queue->enqueueMigrateMemObjects({*inputBuffer, *decompressSizeBuffer}, 0);
        queue->finish();

        queue->enqueueTask(*compressKernel);
        queue->finish();

        queue->enqueueMigrateMemObjects({*(outputBuffer), *(compressedSizeBuffer)}, CL_MIGRATE_MEM_OBJECT_HOST);
        queue->finish();

        uint32_t idx = 0;
        for (uint32_t bIdx=0;bIdx<numBlocks;bIdx++, idx+=block_size_in_bytes) {
            uint32_t blockSize=block_size_in_bytes;
            if (idx+blockSize>chunkSize) blockSize=chunkSize-idx;

            uint32_t compressSize=compressedSizeBufferHost[bIdx];
            assert(compressSize!=0);

            uint32_t percCal=chunkSize * 10;
            percCal=percCal/blockSize;
            if (compressSize<blockSize && percCal>=10) {
                // Chunk Type Identifier
				uint identifier=0x00;
                outputStream.push(&identifier, 1, false);

                // 3 Bytes to represent compress block length + 4;
                uint32_t fixedCompressSize=compressSize+4;
                outputStream.push(&fixedCompressSize, 3, false);

                // CRC - for now 0s
                uint32_t crcValue=0;
                outputStream.push(&crcValue, 4, false);

                // Compressed data of this block with preamble
                if(bIdx==numBlocks-1) outputStream.push(outputBufferHost.data()+idx, compressSize, last);
                else outputStream.push(outputBufferHost.data()+idx, compressSize, false);
            } else {
                // Chunk Type Identifier
				uint identifier=0x01;
                outputStream.push(&identifier, 1, false);

                // 3 Bytes to represent uncompress block length + 4;
                uint32_t fixedCompressSize=blockSize+4;
                outputStream.push(&fixedCompressSize, 3, false);

                // CRC -for now 0s
                uint32_t crcValue=0;
                outputStream.push(&crcValue, 4, false);

                // Uncompressed data copy
                if(bIdx==numBlocks-1) outputStream.push(inputBufferHost.data()+idx, blockSize, last);
                else outputStream.push(outputBufferHost.data()+idx, blockSize, false);
            }
        }
    }while(!last);
}

SnappyCompressWorkshop::SnappyCompressWorkshop() : 
	Workshop("SnappyInputStream", 4, "SnappyOutputStream", 16),
	processThread(&SnappyCompressWorkshop::process, this){}

void SnappyCompressWorkshop::wait(){
	processThread.join();
}

ByteStream& SnappyCompressWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& SnappyCompressWorkshop::getOutputStream(){
	return outputStream;
}