#include "ZstdCompress.hpp"

void ZstdCompressWorkshop::process(){
	cl_int err;

    // host allocated aligned memory
    std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(CHUNK_SIZE_IN_BYTE);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(CHUNK_SIZE_IN_BYTE);
    std::vector<uint32_t, aligned_allocator<uint32_t>> outSizeBufferHost(1);

    BufferPointer inBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, CHUNK_SIZE_IN_BYTE, inBufferHost.data());
    BufferPointer outBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, CHUNK_SIZE_IN_BYTE, outBufferHost.data());
    BufferPointer outSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), outSizeBufferHost.data());

    // set consistent kernel arguments
    KernelPointer cmp_dm_kernel(Application::getProgram<Lib::ZSTD>(), "xilZstdCompressDataMover:{xilZstdCompressDataMover_1}");
    cmp_dm_kernel->setArg(0, *inBuffer);
    cmp_dm_kernel->setArg(1, *outBuffer);
    cmp_dm_kernel->setArg(3, *outSizeBuffer);

    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);

    // auto enbytes = 0;
    // auto outIdx = 0;
    bool last=false;
    // Process the data serially
    // for (uint64_t inIdx=0;inIdx<inputSize;inIdx+=CHUNK_SIZE_IN_BYTE) {
    do{
        // uint32_t chunkSize=CHUNK_SIZE_IN_BYTE;
        // if (inIdx+chunkSize>inputSize) chunkSize=inputSize-inIdx;
        // std::cout<<"index: "<<inIdx<<std::endl;

        // Copy input data
        // std::memcpy(inBufferHost.data(), in+inIdx, chunkSize);
        uint32_t chunkSize=inputStream.pop(inBufferHost.data(), CHUNK_SIZE_IN_BYTE, last);
        // std::cout<<"last: "<<last<<std::endl;
        // std::cout<<"get "<<chunkSize<<" [Bytes]"<<std::endl;
        // hexdump(inBufferHost.data(), chunkSize);

        // Set Variable Kernel Args
        cmp_dm_kernel->setArg(2, chunkSize);

        // Transfer the data to device
        OCL_CHECK(err, err = queue->enqueueMigrateMemObjects({*inBuffer}, 0));
        queue->finish();

        // printf("Free running kernel\n");
        queue->enqueueTask(*cmp_dm_kernel);
        queue->finish();

        // Transfer the data from device to host
        OCL_CHECK(err, err = queue->enqueueMigrateMemObjects({*outBuffer, *outSizeBuffer}, CL_MIGRATE_MEM_OBJECT_HOST));
        queue->finish();

        // std::cout<<"pushed "<<outSizeBufferHost[0]<<" [Bytes], last: "<<last<<std::endl;
        outputStream.push(outBufferHost.data(), outSizeBufferHost[0], last);
        // auto compSize = outSizeBufferHost[0];
        // std::memcpy(out+outIdx, outBufferHost.data(), compSize);
        // enbytes += compSize;
        // outIdx += compSize;
    }while(!last);

    // return enbytes;
}

ZstdCompressWorkshop::ZstdCompressWorkshop() : 
	Workshop("ZstdInputStream", 4, "ZstdOutputStream", 8),
	processThread(&ZstdCompressWorkshop::process, this){}

void ZstdCompressWorkshop::wait(){
	processThread.join();
}

ByteStream& ZstdCompressWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& ZstdCompressWorkshop::getOutputStream(){
	return outputStream;
}