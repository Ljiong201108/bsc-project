#include "Zstd.hpp"

namespace dataCompression{
namespace internalZstd{
uint64_t zstdCompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize){
    cl_int err;

    // host allocated aligned memory
    std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(CHUNK_SIZE_IN_BYTE);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(CHUNK_SIZE_IN_BYTE);
    std::vector<uint32_t, aligned_allocator<uint32_t>> outSizeBufferHost(1);

    BufferPointer inBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, CHUNK_SIZE_IN_BYTE, inBufferHost.data());
    BufferPointer outBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, CHUNK_SIZE_IN_BYTE, outBufferHost.data());
    BufferPointer outSizeBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), outSizeBufferHost.data());

    // set consistent kernel arguments
    KernelPointer cmp_dm_kernel(Application::getProgram<Lib::ZSTD>(), "xilZstdCompressDataMover:{xilZstdCompressDataMover_1}");
    cmp_dm_kernel->setArg(0, *inBuffer);
    cmp_dm_kernel->setArg(1, *outBuffer);
    cmp_dm_kernel->setArg(3, *outSizeBuffer);

    CommandQueuePointer queue(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);

    auto enbytes = 0;
    auto outIdx = 0;
    // Process the data serially
    for (uint64_t inIdx=0;inIdx<inputSize;inIdx+=CHUNK_SIZE_IN_BYTE) {
        uint32_t chunkSize=CHUNK_SIZE_IN_BYTE;
        if (inIdx+chunkSize>inputSize) chunkSize=inputSize-inIdx;
        std::cout<<"index: "<<inIdx<<std::endl;

        // Copy input data
        std::memcpy(inBufferHost.data(), in+inIdx, chunkSize);

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

        auto compSize = outSizeBufferHost[0];
        std::memcpy(out+outIdx, outBufferHost.data(), compSize);
        enbytes += compSize;
        outIdx += compSize;
    }

    return enbytes;
}

uint64_t zstdDecompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize){
    uint64_t outIdx=0;

    std::thread t([]{
        CommandQueuePointer queue(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);
        KernelPointer kernel(Application::getProgram<Lib::ZSTD>(), "xilZstdDecompressStream:{xilZstdDecompressStream_1}");

        queue->enqueueTask(*kernel);
    });

    std::thread chunkWriter([in, inputSize]{
        CommandQueuePointer chunkWriterQueue(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);
        KernelPointer chunkWriterKernel(Application::getProgram<Lib::ZSTD>(), "xilMM2S:{xilMM2S_1}");

        // host allocated aligned memory
        std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(CHUNK_SIZE_IN_BYTE);
        BufferPointer inputBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, CHUNK_SIZE_IN_BYTE, inputBufferHost.data());
        
        chunkWriterKernel->setArg(0, *inputBuffer);

        uint64_t chunkNum=inputSize/CHUNK_SIZE_IN_BYTE+(inputSize%CHUNK_SIZE_IN_BYTE!=0);
        for(uint64_t i=0;i<chunkNum;i++){
            uint32_t chunkSize=std::min(inputSize-i*CHUNK_SIZE_IN_BYTE, CHUNK_SIZE_IN_BYTE);
            std::cout<<"current chunk: "<<i<<std::endl;
            std::cout<<"start to write a chunk["<<chunkSize<<" Bytes]"<<std::endl;
            std::memcpy(inputBufferHost.data(), in+i*CHUNK_SIZE_IN_BYTE, chunkSize); 

            chunkWriterQueue->enqueueMigrateMemObjects({*inputBuffer}, 0);
            chunkWriterQueue->finish();

            chunkWriterKernel->setArg(1, chunkSize);
            if(i==chunkNum-1) chunkWriterKernel->setArg(2, (uint32_t)true);
            else chunkWriterKernel->setArg(2, (uint32_t)false);

            chunkWriterQueue->enqueueTask(*chunkWriterKernel);
            chunkWriterQueue->finish();

            std::cout<<"write a chunk["<<chunkSize<<" Bytes]"<<std::endl;
        }
    });

    std::thread chunkReader([out, &outIdx]{
        CommandQueuePointer chunkReaderQueue(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);
        KernelPointer chunkReaderKernel(Application::getProgram<Lib::ZSTD>(), "xilS2MM:{xilS2MM_1}");

        std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(CHUNK_SIZE_IN_BYTE*2);
        std::vector<uint32_t, aligned_allocator<uint32_t>> outputSizeBufferHost(1);
        std::vector<uint32_t, aligned_allocator<uint32_t>> statusBufferHost(1);
        statusBufferHost[0]=0;

        BufferPointer outputBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint8_t)*outputBufferHost.size(), outputBufferHost.data());
        BufferPointer outputSizeBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), outputSizeBufferHost.data());
        BufferPointer statusBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), statusBufferHost.data());

        chunkReaderKernel->setArg(0, *outputBuffer);
        chunkReaderKernel->setArg(1, *outputSizeBuffer);
        chunkReaderKernel->setArg(2, *statusBuffer);
        chunkReaderKernel->setArg(3, (uint32_t)outputBufferHost.size());

        while(!statusBufferHost[0]){
            std::cout<<"start to read a chunk["<<outputBufferHost.size()<<" Bytes]"<<std::endl;
            chunkReaderQueue->enqueueTask(*chunkReaderKernel);

            chunkReaderQueue->enqueueMigrateMemObjects({*outputSizeBuffer, *statusBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
            chunkReaderQueue->finish();

            chunkReaderQueue->enqueueMigrateMemObjects({*outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
            chunkReaderQueue->finish();
            memcpy(out+outIdx, outputBufferHost.data(), outputSizeBufferHost[0]);

            outIdx+=outputSizeBufferHost[0];
            std::cout<<"read a chunk["<<outputSizeBufferHost[0]<<" Bytes]"<<std::endl;
            std::cout<<"current OutIdx: "<<outIdx<<std::endl;
        }
    });

    chunkWriter.join();
    chunkReader.join();
    t.join();

    return outIdx;
}
}

uint64_t zstdCompress(uint8_t* in, uint8_t* out, uint64_t inputSize){
    uint64_t enbytes = internalZstd::zstdCompressEngineStream(in, out, inputSize);
    return enbytes;
}

uint64_t zstdDecompress(uint8_t* in, uint8_t* out, uint64_t inputSize){
    uint64_t debytes = internalZstd::zstdDecompressEngineStream(in, out, inputSize);
    return debytes;
}
}