#include "Snappy.hpp"

namespace dataCompression{
namespace internalSnappy{
std::mutex compressInputMtx, compressOutputMtx, decompressInputMtx, decompressOutputMtx;
std::condition_variable compressInputCV, compressOutputCV, decompressInputCV, decompressOutputCV;
std::vector<std::unique_ptr<std::thread>> compressInputs, compressOutputs, decompressInputs, decompressOutputs;
std::unique_ptr<std::thread> compressFunc, decompressFunc;
uint32_t compressInputCur, compressOutputCur, decompressInputCur, decompressOutputCur;
uint32_t compressInputIdx, compressOutputIdx, decompressInputIdx, decompressOutputIdx;
uint32_t checksum;

ThreadSafeFIFO<uint8_t> compressQueueInput, compressQueueOutput;
ThreadSafeFIFO<uint8_t> decompressQueueInput, decompressQueueOutput;

uint8_t readHeader(uint8_t* in){
    uint8_t fileIdx = 0;
    uint8_t header_byte = 10;
    fileIdx = header_byte;
    return fileIdx;
}

void snappyCompressionEngine(){
    KernelPointer compressKernel(Application::getProgram<Lib::SNAPPY>(), "xilSnappyCompressMM:{xilSnappyCompressMM_1}");
    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(HOST_BUFFER_SIZE);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(HOST_BUFFER_SIZE);
    std::vector<uint32_t, aligned_allocator<uint32_t>> compressedSizeBufferHost(MAX_NUMBER_BLOCKS);
    std::vector<uint32_t, aligned_allocator<uint32_t>> decompressedSizeBufferHost(MAX_NUMBER_BLOCKS);

    uint32_t block_size_in_kb=BLOCK_SIZE_IN_KB;
    uint32_t block_size_in_bytes=block_size_in_kb*1024;
    bool last;

    // for (uint64_t inIdx = 0; inIdx < inputSize; inIdx += HOST_BUFFER_SIZE) {
    do{
        // uint32_t chunkSize;
        // if (inIdx>inputSize) chunkSize=inputSize-inIdx;
        // else chunkSize = HOST_BUFFER_SIZE;
        // std::memcpy(inputBufferHost.data(), &in[inIdx], chunkSize);
        uint32_t chunkSize=compressQueueInput.pop(inputBufferHost.data(), HOST_BUFFER_SIZE, last);
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
                // out[outIdx++] = 0x00;
                compressQueueOutput.push(0x00, false);

                // 3 Bytes to represent compress block length + 4;
                uint32_t fixedCompressSize=compressSize+4;
                // std::memcpy(&out[outIdx], &fixedCompressSize, 3);
                // outIdx += 3;
                compressQueueOutput.push(&fixedCompressSize, 3, false);

                // CRC - for now 0s
                uint32_t crcValue=0;
                // std::memcpy(&out[outIdx], &crcValue, 4);
                // outIdx += 4;
                compressQueueOutput.push(&crcValue, 4, false);

                // Compressed data of this block with preamble
                // std::memcpy(&out[outIdx], (outputBufferHost.data() + bIdx * block_size_in_bytes), compressSize);
                // outIdx += compressSize;
                if(bIdx==numBlocks-1) compressQueueOutput.push(outputBufferHost.data()+idx, compressSize, last);
                else compressQueueOutput.push(outputBufferHost.data()+idx, compressSize, false);
            } else {
                // Chunk Type Identifier
                // out[outIdx++] = 0x01;
                compressQueueOutput.push(0x01, false);

                // 3 Bytes to represent uncompress block length + 4;
                uint32_t fixedCompressSize=blockSize+4;
                // std::memcpy(&out[outIdx], &fixedCompressSize, 3);
                // outIdx += 3;
                compressQueueOutput.push(&fixedCompressSize, 3, false);

                // CRC -for now 0s
                uint32_t crcValue=0;
                // std::memcpy(&out[outIdx], &crcValue, 4);
                // outIdx += 4;
                compressQueueOutput.push(&crcValue, 4, false);

                // Uncompressed data copy
                // std::memcpy(&out[outIdx], &in[inIdx + (cuCopy * HOST_BUFFER_SIZE) + idx], blockSize);
                // outIdx += blockSize;
                if(bIdx==numBlocks-1) compressQueueOutput.push(inputBufferHost.data()+idx, blockSize, last);
                else compressQueueOutput.push(outputBufferHost.data()+idx, blockSize, false);
            }
        }
    }while(!last);
}

void snappyDecompressionEngine(){
    KernelPointer decompress_kernel_snappy(Application::getProgram<Lib::SNAPPY>(), "xilSnappyDecompressMM:{xilSnappyDecompressMM_1}");
    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(HOST_BUFFER_SIZE);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(HOST_BUFFER_SIZE);
    std::vector<uint32_t, aligned_allocator<uint32_t>> compressedSizeBufferHost(MAX_NUMBER_BLOCKS);
    std::vector<uint32_t, aligned_allocator<uint32_t>> decompressedSizeBufferHost(MAX_NUMBER_BLOCKS);
    std::vector<uint8_t> notCompressedBufferHost(HOST_BUFFER_SIZE);
    std::vector<uint32_t> notCompressedSizeBufferHost(MAX_NUMBER_BLOCKS);
    std::vector<bool> isCompressed(MAX_NUMBER_BLOCKS);

    uint32_t block_size_in_byte=BLOCK_SIZE_IN_KB * 1024;
    uint32_t maxNumBlocks=HOST_BUFFER_SIZE / block_size_in_byte;
    uint32_t blockNum=0, compressedBlockNum=0, notCompressedBlockNum=0;
    bool last;

    BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, HOST_BUFFER_SIZE, inputBufferHost.data());
    BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, HOST_BUFFER_SIZE, outputBufferHost.data());
    BufferPointer decompressSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t)*maxNumBlocks, decompressedSizeBufferHost.data());
    BufferPointer compressedSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t)*maxNumBlocks, compressedSizeBufferHost.data());

    decompress_kernel_snappy->setArg(0, *inputBuffer);
    decompress_kernel_snappy->setArg(1, *outputBuffer);
    decompress_kernel_snappy->setArg(2, *decompressSizeBuffer);
    decompress_kernel_snappy->setArg(3, *compressedSizeBuffer);
    decompress_kernel_snappy->setArg(4, (uint32_t)BLOCK_SIZE_IN_KB);

    decompressQueueInput.pop(inputBufferHost.data(), 10, last);
    uint8_t streamIdentifier[]={0xff, 0x06, 0x00, 0x00, 0x73, 0x4e, 0x61, 0x50, 0x70, 0x59};
    if(std::memcmp(streamIdentifier, inputBufferHost.data(), 10)!=0){
        std::cerr<<"stream identifier invalid!"<<std::endl;
        exit(EXIT_FAILURE);
    }

    while(!last){
        uint8_t chunkIdentifier=decompressQueueInput.pop(last);

        if(chunkIdentifier==0x00){
            uint32_t b1=decompressQueueInput.pop(last);
            uint32_t b2=decompressQueueInput.pop(last);
            uint32_t b3=decompressQueueInput.pop(last);

            uint32_t blockSize=b1+(b2<<8)+(b3<<16);
            std::cout<<"0x00 block with size: "<<blockSize<<std::endl;

            uint32_t checksum;
            decompressQueueInput.pop(&checksum, 4, last);
            blockSize-=4;

            decompressQueueInput.pop(inputBufferHost.data()+compressedBlockNum*block_size_in_byte, blockSize, last);
            compressedSizeBufferHost[compressedBlockNum]=blockSize;

            //copied and modified from xilinx
            uint8_t *curIn=inputBufferHost.data()+compressedBlockNum*block_size_in_byte;
            uint8_t bval1 = curIn[0];
            uint32_t final_size = 0;

            if ((bval1 >> 7) == 1) {
                uint8_t b1 = bval1 & 0x7F;
                bval1 = curIn[1];
                uint8_t b2 = bval1 & 0x7F;
                if ((bval1 >> 7) == 1) {
                    bval1 = curIn[2];
                    uint8_t b3 = bval1 & 0x7F;
                    uint32_t temp1 = b3;
                    temp1 <<= 14;
                    uint32_t temp2 = b2;
                    temp2 <<= 7;
                    uint32_t temp3 = b1;
                    final_size |= temp1;
                    final_size |= temp2;
                    final_size |= temp3;
                } else {
                    uint32_t temp1 = b2;
                    temp1 <<= 7;
                    uint32_t temp2 = b1;
                    final_size |= temp1;
                    final_size |= temp2;
                }
            }
            //end
            decompressedSizeBufferHost[compressedBlockNum]=final_size;
            std::cout<<"final_size: "<<final_size<<std::endl;

            isCompressed[blockNum]=1;
            compressedBlockNum++;
            blockNum++;
        }else if(chunkIdentifier==0x01){
            uint32_t b1=decompressQueueInput.pop(last);
            uint32_t b2=decompressQueueInput.pop(last);
            uint32_t b3=decompressQueueInput.pop(last);

            uint32_t blockSize=b1+(b2<<8)+(b3<<16);
            std::cout<<"0x01 block with size: "<<blockSize<<std::endl;

            uint32_t checksum;
            decompressQueueInput.pop(&checksum, 4, last);
            blockSize-=4;

            decompressQueueInput.pop(notCompressedBufferHost.data()+notCompressedBlockNum*block_size_in_byte, blockSize, last);
            notCompressedSizeBufferHost[notCompressedBlockNum]=blockSize;
            isCompressed[blockNum]=0;
            notCompressedBlockNum++;
            blockNum++;
        }else if(chunkIdentifier==0xff){
            decompressQueueInput.pop(inputBufferHost.data(), 9, last);
        }else{
            std::cerr<<"chunk identifier invalid!"<<std::endl;
            exit(EXIT_FAILURE);
        }

        assert(compressedBlockNum+notCompressedBlockNum==blockNum && "should be equal");
        if(compressedBlockNum+notCompressedBlockNum==maxNumBlocks-1 || last){
            std::cout<<"going to execute the kernel"<<std::endl;
            std::cout<<"compressedBlockNum: "<<compressedBlockNum<<std::endl;
            std::cout<<"notCompressedBlockNum: "<<notCompressedBlockNum<<std::endl;
            std::cout<<"blockNum: "<<blockNum<<std::endl;
            decompress_kernel_snappy->setArg(5, compressedBlockNum);

            queue->enqueueMigrateMemObjects({*inputBuffer, *compressedSizeBuffer}, 0);
            queue->finish();

            queue->enqueueTask(*decompress_kernel_snappy);
            queue->finish();

            queue->enqueueMigrateMemObjects({*outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
            queue->finish();

            // hexdump(outputBufferHost.data(), 65536);

            // hexdump(outputBufferHost.data(), HOST_BUFFER_SIZE);
            for(uint32_t i=0;i<compressedBlockNum;i++){
                std::cout<<i<<" "<<decompressedSizeBufferHost[i]<<std::endl;
            }
            std::cout<<std::endl;

            uint32_t compressedIdx=0, notCompressedIdx=0;
            for(uint32_t blockIdx=0;blockIdx<blockNum;blockIdx++){
                if(isCompressed[blockIdx]){
                    uint32_t blockSize=decompressedSizeBufferHost[compressedIdx];
                    std::cout<<"compressed block: "<<compressedIdx<<", block size is "<<blockSize<<std::endl;
                    decompressQueueOutput.push(outputBufferHost.data()+compressedIdx*block_size_in_byte, blockSize, blockIdx==blockNum-1 ? last : false);
                    compressedIdx++;
                }else{
                    uint32_t blockSize=notCompressedSizeBufferHost[notCompressedIdx];
                    std::cout<<"not compressed block: "<<compressedIdx<<", block size is "<<blockSize<<std::endl;
                    decompressQueueOutput.push(notCompressedBufferHost.data()+notCompressedIdx*block_size_in_byte, blockSize, blockIdx==blockNum-1 ? last : false);
                    notCompressedIdx++;
                }
            }

            //reset everything
            blockNum=0;
            compressedBlockNum=0;
            notCompressedBlockNum=0;
        }
    }
}
}

uint8_t writeSnappyHeader(uint8_t* out){
    int fileIdx = 0;

    // Snappy Stream Identifier
    out[fileIdx++] = 0xff;
    out[fileIdx++] = 0x06;
    out[fileIdx++] = 0x00;
    out[fileIdx++] = 0x00;
    out[fileIdx++] = 0x73;
    out[fileIdx++] = 0x4e;
    out[fileIdx++] = 0x61;
    out[fileIdx++] = 0x50;
    out[fileIdx++] = 0x70;
    out[fileIdx++] = 0x59;

    return fileIdx;
}

uint64_t snappyCompressEngineMM(uint8_t* in, uint8_t* out, uint64_t inputSize){
    KernelPointer compressKernel(Application::getProgram<Lib::SNAPPY>(), "xilSnappyCompressMM:{xilSnappyCompressMM_1}");
    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    std::vector<uint8_t, aligned_allocator<uint8_t> > inputBufferHost(internalSnappy::HOST_BUFFER_SIZE);
    std::vector<uint8_t, aligned_allocator<uint8_t> > outputBufferHost(internalSnappy::HOST_BUFFER_SIZE);
    std::vector<uint32_t, aligned_allocator<uint32_t> > compressedSizeBufferHost(internalSnappy::MAX_NUMBER_BLOCKS);
    std::vector<uint32_t, aligned_allocator<uint32_t> > decompressedSizeBufferHost(internalSnappy::MAX_NUMBER_BLOCKS);

    uint32_t block_size_in_kb = internalSnappy::BLOCK_SIZE_IN_KB;
    uint32_t block_size_in_bytes = block_size_in_kb * 1024;

    // Keeps track of output buffer index
    uint64_t outIdx = 0;

    // Given a input file, we process it as multiple chunks
    // Each compute unit is assigned with a chunk of data
    // In this example HOST_BUFFER_SIZE is the chunk size.
    // For example: Input file = 12 MB
    // HOST_BUFFER_SIZE = 2MB
    // Each compute unit processes 2MB data per kernel invocation
    uint32_t chunkSize;
    // This buffer contains total number of BLOCK_SIZE_IN_KB blocks per CU
    // For Example: HOST_BUFFER_SIZE = 2MB/BLOCK_SIZE_IN_KB = 32block (Block
    // size 64 by default)
    uint32_t total_blocks_cu;

    // This buffer holds exact size of the chunk in bytes for all the CUs
    uint32_t bufferSize;

    // Holds value of total compute units to be
    // used per iteration
    int compute_cu = 0;
    for (uint64_t inIdx = 0; inIdx < inputSize; inIdx += internalSnappy::HOST_BUFFER_SIZE) {
        // Needs to reset this variable
        // As this drives compute unit launch per iteration
        compute_cu = 0;

        // Pick buffer size as predefined one
        // If yet to be consumed input is lesser
        // the reset to required size

        uint32_t buf_size = internalSnappy::HOST_BUFFER_SIZE;

        // This loop traverses through each compute based current inIdx
        // It tries to calculate chunk size and total compute units need to be
        // launched (based on the inputSize)

        for (int bufCalc = 0; bufCalc < 1; bufCalc++) {
            chunkSize = 0;
            // If amount of data to be consumed is less than HOST_BUFFER_SIZE
            // Then choose to send is what is needed instead of full buffer size
            // based on host buffer macro

            if (inIdx + (buf_size * (bufCalc + 1)) > inputSize) {
                chunkSize = inputSize - (inIdx + internalSnappy::HOST_BUFFER_SIZE * bufCalc);
                compute_cu++;
                break;
            } else {
                chunkSize = buf_size;
                compute_cu++;
            }
        }
        // Figure out total number of blocks need per each chunk
        // Copy input data from in to host buffer based on the inIdx and cu

        for (int blkCalc = 0; blkCalc < compute_cu; blkCalc++) {
            uint32_t numBlocks = (chunkSize - 1) / block_size_in_bytes + 1;
            total_blocks_cu = numBlocks;
            std::memcpy(inputBufferHost.data(), &in[inIdx + blkCalc * internalSnappy::HOST_BUFFER_SIZE], chunkSize);
        }
        // Fill the host block size buffer with various block sizes per chunk/cu
        for (int cuBsize = 0; cuBsize < compute_cu; cuBsize++) {
            uint32_t bIdx = 0;
            uint32_t chunkSize_curr_cu = chunkSize;

            for (uint32_t bs = 0; bs < chunkSize_curr_cu; bs += block_size_in_bytes) {
                uint32_t blockSize = block_size_in_bytes;
                if (bs + blockSize > chunkSize_curr_cu) {
                    blockSize = chunkSize_curr_cu - bs;
                }
                decompressedSizeBufferHost.data()[bIdx++] = blockSize;
            }
            // Calculate chunks size in bytes for device buffer creation
            bufferSize = ((chunkSize - 1) / internalSnappy::BLOCK_SIZE_IN_KB + 1) * internalSnappy::BLOCK_SIZE_IN_KB;
        }

        BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, bufferSize, inputBufferHost.data());
        BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, bufferSize, outputBufferHost.data());
        BufferPointer compressedSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t) * total_blocks_cu, compressedSizeBufferHost.data());
        BufferPointer decompressSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t) * total_blocks_cu, decompressedSizeBufferHost.data());

        int narg = 0;
        compressKernel->setArg(narg++, *(inputBuffer));
        compressKernel->setArg(narg++, *(outputBuffer));
        compressKernel->setArg(narg++, *(compressedSizeBuffer));
        compressKernel->setArg(narg++, *(decompressSizeBuffer));
        compressKernel->setArg(narg++, block_size_in_kb);
        compressKernel->setArg(narg++, chunkSize);
        std::vector<cl::Memory> inBufVec;

        inBufVec.push_back(*(inputBuffer));
        inBufVec.push_back(*(decompressSizeBuffer));

        // Migrate memory - Map host to device buffers
        queue->enqueueMigrateMemObjects(inBufVec, 0 /* 0 means from host*/);
        queue->finish();

        // Fire kernel execution
        queue->enqueueTask(*compressKernel);
        // wait till kernels complete
        queue->finish();

        // Migrate memory - Map device to host buffers
        queue->enqueueMigrateMemObjects({*(outputBuffer), *(compressedSizeBuffer)}, CL_MIGRATE_MEM_OBJECT_HOST);
        queue->finish();
        for (int cuCopy = 0; cuCopy < compute_cu; cuCopy++) {
            // Copy data into out buffer
            // Include compress and block size data
            // Copy data block by block within a chunk example 2MB (64block size) - 32 blocks data
            // Do the same for all the compute units
            uint32_t idx = 0;
            for (uint32_t bIdx = 0; bIdx < total_blocks_cu; bIdx++, idx += block_size_in_bytes) {
                // Default block size in bytes i.e., 64 * 1024
                uint32_t blockSize = block_size_in_bytes;
                if (idx + blockSize > chunkSize) {
                    blockSize = chunkSize - idx;
                }
                uint32_t compressSize = compressedSizeBufferHost.data()[bIdx];
                assert(compressSize != 0);

                int orig_block_size = chunkSize;
                int percCal = orig_block_size * 10;
                percCal = percCal / blockSize;
                if (compressSize < blockSize && percCal >= 10) {
                    // Chunk Type Identifier
                    out[outIdx++] = 0x00;
                    // 3 Bytes to represent compress block length + 4;
                    uint32_t fixedCompressSize = compressSize + 4;
                    std::memcpy(&out[outIdx], &fixedCompressSize, 3);
                    outIdx += 3;
                    // CRC - for now 0s
                    uint32_t crcValue = 0;
                    std::memcpy(&out[outIdx], &crcValue, 4);
                    outIdx += 4;
                    // Compressed data of this block with preamble
                    std::memcpy(&out[outIdx], (outputBufferHost.data() + bIdx * block_size_in_bytes), compressSize);
                    outIdx += compressSize;
                } else {
                    // Chunk Type Identifier
                    out[outIdx++] = 0x01;
                    // 3 Bytes to represent uncompress block length + 4;
                    uint32_t fixedCompressSize = blockSize + 4;
                    std::memcpy(&out[outIdx], &fixedCompressSize, 3);
                    outIdx += 3;
                    // CRC -for now 0s
                    uint32_t crcValue = 0;
                    std::memcpy(&out[outIdx], &crcValue, 4);
                    outIdx += 4;

                    // Uncompressed data copy
                    std::memcpy(&out[outIdx], &in[inIdx + (cuCopy * internalSnappy::HOST_BUFFER_SIZE) + idx], blockSize);
                    outIdx += blockSize;
                } // End of else - uncompressed stream update
            }     // End of chunk (block by block) copy to output buffer
        }         // End of CU loop - Each CU/chunk block by block copy
                  // Buffer deleted
    }
    return outIdx;
}

uint64_t snappyCompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize){
    uint64_t hostBufferSize = internalSnappy::BLOCK_SIZE_IN_KB * 1024;
    uint64_t chunkNum = (inputSize - 1) / hostBufferSize + 1;

    // output buffer index
    uint64_t outIdx = 0;
    // Index calculation
    std::vector<uint8_t, aligned_allocator<uint8_t> > inputBufferHost(hostBufferSize);
    std::vector<uint8_t, aligned_allocator<uint8_t> > outputBufferHost(hostBufferSize);
    std::vector<uint32_t, aligned_allocator<uint32_t> > compressedSizeBufferHost(1);

    // device buffer allocation
    BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, hostBufferSize, inputBufferHost.data());
    BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, hostBufferSize, outputBufferHost.data());
    BufferPointer compressedSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), compressedSizeBufferHost.data());

    KernelPointer compressKernel(Application::getProgram<Lib::SNAPPY>(), "xilSnappyCompressStream:{xilSnappyCompressStream_1}");
    KernelPointer compressDataMoverKernel(Application::getProgram<Lib::SNAPPY>(), "xilCompressDatamover:{xilCompressDatamover_1}");
    compressDataMoverKernel->setArg(0, *inputBuffer);
    compressDataMoverKernel->setArg(1, *outputBuffer);
    compressDataMoverKernel->setArg(2, *compressedSizeBuffer);

    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    for (uint64_t blkIndx=0, bufIndx=0; blkIndx<chunkNum; blkIndx++, bufIndx+=hostBufferSize) {
        // current block input size
        uint32_t chunkSize = hostBufferSize;
        if (blkIndx==chunkNum-1) chunkSize=inputSize-bufIndx;

        // copy input to input buffer
        std::memcpy(inputBufferHost.data(), in+bufIndx, chunkSize);

        // set kernel args
        compressDataMoverKernel->setArg(3, chunkSize);

        compressKernel->setArg(2, chunkSize);
        // Migrate Memory - Map host to device buffers
        queue->enqueueMigrateMemObjects({*inputBuffer}, 0);
        queue->finish();
        
        // enqueue the kernels and wait for them to finish
        queue->enqueueTask(*compressDataMoverKernel);
        queue->enqueueTask(*compressKernel);
        queue->finish();

        // Migrate memory - Map device to host buffers
        queue->enqueueMigrateMemObjects({*outputBuffer, *compressedSizeBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
        queue->finish();

        // copy the compressed data to out pointer
        uint32_t compressedSize=compressedSizeBufferHost.data()[0];

        if (chunkSize>compressedSize) {
            out[outIdx++]=0x00;

            // 3 Bytes to represent compressed block length + 4
            uint32_t fixedSize=compressedSize+4;
            std::memcpy(out+outIdx, &fixedSize, 3);
            outIdx+=3;

            // CRC - for now 0s
            uint32_t crcValue=0;
            std::memcpy(out+outIdx, &crcValue, 4);
            outIdx+=4;

            std::memcpy(out+outIdx, outputBufferHost.data(), compressedSize);
            outIdx+=compressedSize;
        } else {
            // Chunk Type Identifier
            out[outIdx++]=0x01;
            // 3 Bytes to represent uncompress block length + 4;
            uint32_t fixedSize=chunkSize + 4;
            std::memcpy(out+outIdx, &fixedSize, 3);
            outIdx+=3;
            // CRC -for now 0s
            uint32_t crcValue=0;
            std::memcpy(out+outIdx, &crcValue, 4);
            outIdx+=4;

            std::memcpy(out+outIdx, in+(hostBufferSize*blkIndx), chunkSize);
            outIdx+=chunkSize;
        }
    }

    return outIdx;
}

uint64_t snappyDecompressEngineMM(uint8_t* in, uint8_t* out, uint64_t inputSize){
    KernelPointer decompress_kernel_snappy(Application::getProgram<Lib::SNAPPY>(), "xilSnappyDecompressMM:{xilSnappyDecompressMM_1}");
    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    std::vector<uint8_t, aligned_allocator<uint8_t> > inputBufferHost(internalSnappy::HOST_BUFFER_SIZE);
    std::vector<uint8_t, aligned_allocator<uint8_t> > outputBufferHost(internalSnappy::HOST_BUFFER_SIZE);
    std::vector<uint32_t, aligned_allocator<uint32_t> > compressedSizeBufferHost(internalSnappy::MAX_NUMBER_BLOCKS);
    std::vector<uint32_t, aligned_allocator<uint32_t> > decompressedSizeBufferHost(internalSnappy::MAX_NUMBER_BLOCKS);
    std::vector<uint32_t> m_blkSize(internalSnappy::MAX_NUMBER_BLOCKS);
    std::vector<uint32_t> m_compressSize(internalSnappy::MAX_NUMBER_BLOCKS);

    uint32_t buf_size = internalSnappy::BLOCK_SIZE_IN_KB * 1024;
    uint32_t blocksPerChunk = internalSnappy::HOST_BUFFER_SIZE / buf_size;
    uint32_t host_buffer_size = ((internalSnappy::HOST_BUFFER_SIZE - 1) / internalSnappy::BLOCK_SIZE_IN_KB + 1) * internalSnappy::BLOCK_SIZE_IN_KB;

    BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, host_buffer_size, inputBufferHost.data());
    BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, host_buffer_size, outputBufferHost.data());
    BufferPointer decompressSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t) * blocksPerChunk, decompressedSizeBufferHost.data());
    BufferPointer compressedSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t) * blocksPerChunk, compressedSizeBufferHost.data());
                                        
    uint32_t narg = 0;
    decompress_kernel_snappy->setArg(narg++, *(inputBuffer));
    decompress_kernel_snappy->setArg(narg++, *(outputBuffer));
    decompress_kernel_snappy->setArg(narg++, *(decompressSizeBuffer));
    decompress_kernel_snappy->setArg(narg++, *(compressedSizeBuffer));
    decompress_kernel_snappy->setArg(narg++, (uint32_t)internalSnappy::BLOCK_SIZE_IN_KB);
    decompress_kernel_snappy->setArg(narg++, blocksPerChunk);
    uint32_t chunk_size = 0;
    uint8_t chunk_idx = 0;
    uint32_t block_cntr = 0;
    uint32_t blockSize = 0;
    uint32_t chunk_cntr = 0;
    uint32_t bufblocks = 0;
    uint64_t output_idx = 0;
    uint32_t bufIdx = 0;
    uint32_t over_block_cntr = 0;
    uint32_t brick = 0;
    uint16_t stride_cidsize = 4;
    bool blkDecomExist = false;
    uint32_t blkUnComp = 0;
    
    // Go over overall input size
    for (uint32_t idxSize = 0; idxSize < inputSize; idxSize += stride_cidsize, chunk_cntr++) {
        // Chunk identifier
        chunk_idx = in[idxSize];
        chunk_size = 0;
        // Chunk Compressed size
        uint8_t cbyte_1 = in[idxSize + 1];
        uint8_t cbyte_2 = in[idxSize + 2];
        uint8_t cbyte_3 = in[idxSize + 3];

        uint32_t temp = cbyte_3;
        temp <<= 16;
        chunk_size |= temp;
        temp = 0;
        temp = cbyte_2;
        temp <<= 8;
        chunk_size |= temp;
        temp = 0;
        chunk_size |= cbyte_1;

        if (chunk_idx == 0x00) {
            uint8_t bval1 = in[idxSize + 8];
            uint32_t final_size = 0;

            if ((bval1 >> 7) == 1) {
                uint8_t b1 = bval1 & 0x7F;
                bval1 = in[idxSize + 9];
                uint8_t b2 = bval1 & 0x7F;
                if ((bval1 >> 7) == 1) {
                    bval1 = in[idxSize + 10];
                    uint8_t b3 = bval1 & 0x7F;
                    uint32_t temp1 = b3;
                    temp1 <<= 14;
                    uint32_t temp2 = b2;
                    temp2 <<= 7;
                    uint32_t temp3 = b1;
                    final_size |= temp1;
                    final_size |= temp2;
                    final_size |= temp3;
                } else {
                    uint32_t temp1 = b2;
                    temp1 <<= 7;
                    uint32_t temp2 = b1;
                    final_size |= temp1;
                    final_size |= temp2;
                }

                blockSize = final_size;
            } else {
                blockSize = bval1;
            }
            m_compressSize.data()[over_block_cntr] = chunk_size - 4;
            m_blkSize.data()[over_block_cntr] = blockSize;

            compressedSizeBufferHost.data()[bufblocks] = chunk_size - 4;
            decompressedSizeBufferHost.data()[bufblocks] = blockSize;
            bufblocks++;
            // Copy data
            std::memcpy(&(inputBufferHost.data()[block_cntr * buf_size]), &in[idxSize + 8], chunk_size - 4);
            block_cntr++;
            blkDecomExist = true;
        } else if (chunk_idx == 0x01) {
            m_compressSize.data()[over_block_cntr] = chunk_size - 4;
            m_blkSize.data()[over_block_cntr] = chunk_size - 4;
            std::memcpy(&out[brick * internalSnappy::HOST_BUFFER_SIZE + over_block_cntr * buf_size], &in[idxSize + 8], chunk_size - 4);
            blkUnComp += chunk_size - 4;
        }

        over_block_cntr++;
        // Increment the input idx to
        // compressed size length
        idxSize += chunk_size;

        if (over_block_cntr == blocksPerChunk && blkDecomExist) {
            blkDecomExist = false;
            // Track the chunks processed
            brick++;
            // In case of left over set kernel arg to no blocks
            decompress_kernel_snappy->setArg(5, block_cntr);
            // For big files go ahead do it here
            // Migrate memory - Map host to device buffers
            queue->enqueueMigrateMemObjects({*(inputBuffer), *(decompressSizeBuffer), *(compressedSizeBuffer)}, 0 /*0 means from host*/);
            queue->finish();

            queue->enqueueTask(*decompress_kernel_snappy);
            queue->finish();

            // Migrate memory - Map device to host buffers
            queue->enqueueMigrateMemObjects({*(outputBuffer)}, CL_MIGRATE_MEM_OBJECT_HOST);
            queue->finish();
            bufIdx = 0;
            // copy output
            for (uint32_t bIdx = 0; bIdx < over_block_cntr; bIdx++) {
                uint32_t blockSize = m_blkSize.data()[bIdx];
                uint32_t compressSize = m_compressSize.data()[bIdx];

                if (compressSize < blockSize) {
                    std::memcpy(&out[output_idx], &outputBufferHost.data()[bufIdx], blockSize);
                    output_idx += blockSize;
                    bufIdx += blockSize;
                } else if (compressSize == blockSize) {
                    output_idx += blockSize;
                    blkUnComp -= blockSize;
                }
            }
            block_cntr = 0;
            bufblocks = 0;
            over_block_cntr = 0;
        } else if (over_block_cntr == blocksPerChunk) {
            over_block_cntr = 0;
            brick++;
            bufblocks = 0;
            block_cntr = 0;
        }
    }

    if (block_cntr != 0) {
        // In case of left over set kernel arg to no blocks
        decompress_kernel_snappy->setArg(5, block_cntr);

        // Migrate memory - Map host to device buffers
        queue->enqueueMigrateMemObjects({*(inputBuffer), *(decompressSizeBuffer), *(compressedSizeBuffer)}, 0 /*0 means from host*/);
        queue->finish();
        
        // Kernel invocation
        queue->enqueueTask(*decompress_kernel_snappy);
        queue->finish();

        // Migrate memory - Map device to host buffers
        queue->enqueueMigrateMemObjects({*(outputBuffer)}, CL_MIGRATE_MEM_OBJECT_HOST);
        queue->finish();
        bufIdx = 0;
        // copy output
        for (uint32_t bIdx = 0; bIdx < over_block_cntr; bIdx++) {
            uint32_t blockSize = m_blkSize.data()[bIdx];
            uint32_t compressSize = m_compressSize.data()[bIdx];

            if (compressSize < blockSize) {
                std::memcpy(&out[output_idx], &outputBufferHost.data()[bufIdx], blockSize);
                output_idx += blockSize;
                bufIdx += blockSize;
            } else if (compressSize == blockSize) {
                output_idx += blockSize;
                blkUnComp -= blockSize;
            }
        }

    } // If to see if tehr eare some blocks to be processed
    if (output_idx == 0 && blkUnComp != 0) {
        output_idx = blkUnComp;
    }

    return output_idx;
}

uint64_t snappyDecompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize){
    KernelPointer decompress_kernel_snappy(Application::getProgram<Lib::SNAPPY>(), "xilSnappyDecompressStream:{xilSnappyDecompressStream_1}");
    KernelPointer decompress_data_mover_kernel(Application::getProgram<Lib::SNAPPY>(), "xilDecompressDatamover:{xilDecompressDatamover_1}");
    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    uint32_t outputSize = inputSize*2; // (inputSize * 20) + 16; //m_maxCR
    std::vector<uint8_t, aligned_allocator<uint8_t> > inputBufferHost(inputSize);
    std::vector<uint8_t, aligned_allocator<uint8_t> > outputBufferHost(outputSize);
    std::vector<uint32_t, aligned_allocator<uint32_t> > h_buf_decompressSize(1);

    std::memcpy(inputBufferHost.data(), in, inputSize);

    // Device buffer allocation
    BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, inputSize, inputBufferHost.data());
    BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, outputSize, outputBufferHost.data());
    BufferPointer bufferOutputSize(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), h_buf_decompressSize.data());

    // set kernel arguments
    int narg = 0;
    decompress_data_mover_kernel->setArg(narg++, *(inputBuffer));
    decompress_data_mover_kernel->setArg(narg++, *(outputBuffer));
    decompress_data_mover_kernel->setArg(narg++, inputSize);
    decompress_data_mover_kernel->setArg(narg, *(bufferOutputSize));

    decompress_kernel_snappy->setArg(3, inputSize);

    // Migrate Memory - Map host to device buffers
    queue->enqueueMigrateMemObjects({*(inputBuffer), *(bufferOutputSize), *(outputBuffer)}, 0);
    queue->finish();

    // enqueue the kernels and wait for them to finish
    queue->enqueueTask(*decompress_data_mover_kernel);
    queue->enqueueTask(*decompress_kernel_snappy);
    queue->finish();
    
    // Migrate memory - Map device to host buffers
    queue->enqueueMigrateMemObjects({*(outputBuffer), *(bufferOutputSize)}, CL_MIGRATE_MEM_OBJECT_HOST);
    queue->finish();

    uint32_t uncompressedSize = h_buf_decompressSize[0];
    std::memcpy(out, outputBufferHost.data(), uncompressedSize);

    return uncompressedSize;
}

void snappyCompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last){
    if(internalSnappy::compressFunc==nullptr){
        internalSnappy::compressFunc.reset(nullptr);
        internalSnappy::compressFunc=std::make_unique<std::thread>(&internalSnappy::snappyCompressionEngine);

        internalSnappy::compressInputs.resize(0);
        internalSnappy::compressInputCur=internalSnappy::compressInputIdx=0;
    }

    uint32_t compressInputIdx=internalSnappy::compressInputIdx++;
    internalSnappy::compressInputs.push_back(std::make_unique<std::thread>([in, inputSize, last, compressInputIdx]{
        std::cout<<"write thread "<<compressInputIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalSnappy::compressInputMtx);
        internalSnappy::compressInputCV.wait(lck, [compressInputIdx] {return internalSnappy::compressInputCur==compressInputIdx;});

        std::cout<<"write thread "<<compressInputIdx<<" start to write"<<std::endl;
        internalSnappy::compressQueueInput.push(in, inputSize, last);

        internalSnappy::compressInputCur++;
        internalSnappy::compressInputCV.notify_all();
        std::cout<<"write thread "<<compressInputIdx<<" finished"<<std::endl;
    }));
}

void snappyCompressionInput(uint8_t *in, uint32_t inputSize, bool last){
    if(internalSnappy::compressFunc==nullptr){
        internalSnappy::compressFunc.reset(nullptr);
        internalSnappy::compressFunc=std::make_unique<std::thread>(&internalSnappy::snappyCompressionEngine);
    }

    internalSnappy::compressQueueInput.push(in, inputSize, last);
}

uint32_t snappyCompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size;

    uint32_t compressOutputIdx=internalSnappy::compressOutputIdx++;
    internalSnappy::compressOutputs.push_back(std::make_unique<std::thread>([out, outputSize, &last, compressOutputIdx, &size]{
        std::cout<<"write thread "<<compressOutputIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalSnappy::compressOutputMtx);
        internalSnappy::compressOutputCV.wait(lck, [compressOutputIdx] {return internalSnappy::compressOutputCur==compressOutputIdx;});

        std::cout<<"write thread "<<compressOutputIdx<<" start to write"<<std::endl;
        size=internalSnappy::compressQueueOutput.pop(out, outputSize, last);

        internalSnappy::compressOutputCur++;
        internalSnappy::compressOutputCV.notify_all();
        std::cout<<"write thread "<<compressOutputIdx<<" finished"<<std::endl;
    }));

    if(last){
        for(auto &t : internalSnappy::compressInputs) t->join();
        for(auto &t : internalSnappy::compressOutputs) t->join();
        internalSnappy::compressFunc->join();

        internalSnappy::compressFunc.reset(nullptr);
        internalSnappy::compressInputs.resize(0);
        internalSnappy::compressOutputs.resize(0);
        internalSnappy::compressInputCur=internalSnappy::compressInputIdx=0;
        internalSnappy::compressOutputCur=internalSnappy::compressOutputIdx=0;
    }

    return size;
}

uint32_t snappyCompressionOutput(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size=internalSnappy::compressQueueOutput.pop(out, outputSize, last);

    if(last){
        internalSnappy::compressFunc->join();
        internalSnappy::compressFunc.reset(nullptr);
    }

    return size;
}

void snappyDecompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last){
    if(internalSnappy::decompressFunc==nullptr){
        internalSnappy::decompressFunc.reset(nullptr);
        internalSnappy::decompressFunc=std::make_unique<std::thread>(&internalSnappy::snappyDecompressionEngine);

        internalSnappy::decompressInputs.resize(0);
        internalSnappy::decompressInputCur=internalSnappy::decompressInputIdx=0;
    }

    uint32_t decompressInputIdx=internalSnappy::decompressInputIdx++;
    internalSnappy::decompressInputs.push_back(std::make_unique<std::thread>([in, inputSize, last, decompressInputIdx]{
        std::cout<<"write thread "<<decompressInputIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalSnappy::decompressInputMtx);
        internalSnappy::decompressInputCV.wait(lck, [decompressInputIdx] {return internalSnappy::decompressInputCur==decompressInputIdx;});

        std::cout<<"write thread "<<decompressInputIdx<<" start to write"<<std::endl;
        internalSnappy::decompressQueueInput.push(in, inputSize, last);

        internalSnappy::decompressInputCur++;
        internalSnappy::decompressInputCV.notify_all();
        std::cout<<"write thread "<<decompressInputIdx<<" finished"<<std::endl;
    }));
}

void snappyDecompressionInput(uint8_t *in, uint32_t inputSize, bool last){
    if(internalSnappy::decompressFunc==nullptr){
        internalSnappy::decompressFunc.reset(nullptr);
        internalSnappy::decompressFunc=std::make_unique<std::thread>(&internalSnappy::snappyDecompressionEngine);
    }

    internalSnappy::decompressQueueInput.push(in, inputSize, last);
}

uint32_t snappyDecompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size;

    uint32_t decompressOutputIdx=internalSnappy::decompressOutputIdx++;
    internalSnappy::decompressOutputs.push_back(std::make_unique<std::thread>([out, outputSize, &last, decompressOutputIdx, &size]{
        std::cout<<"write thread "<<decompressOutputIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalSnappy::decompressOutputMtx);
        internalSnappy::decompressOutputCV.wait(lck, [decompressOutputIdx] {return internalSnappy::decompressOutputCur==decompressOutputIdx;});

        std::cout<<"write thread "<<decompressOutputIdx<<" start to write"<<std::endl;
        size=internalSnappy::decompressQueueOutput.pop(out, outputSize, last);

        internalSnappy::decompressOutputCur++;
        internalSnappy::decompressOutputCV.notify_all();
        std::cout<<"write thread "<<decompressOutputIdx<<" finished"<<std::endl;
    }));

    if(last){
        for(auto &t : internalSnappy::decompressInputs) t->join();
        for(auto &t : internalSnappy::decompressOutputs) t->join();
        internalSnappy::decompressFunc->join();

        internalSnappy::decompressFunc.reset(nullptr);
        internalSnappy::decompressInputs.resize(0);
        internalSnappy::decompressOutputs.resize(0);
        internalSnappy::decompressInputCur=internalSnappy::decompressInputIdx=0;
        internalSnappy::decompressOutputCur=internalSnappy::decompressOutputIdx=0;
    }

    return size;
}

uint32_t snappyDecompressionOutput(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size=internalSnappy::decompressQueueOutput.pop(out, outputSize, last);

    if(last){
        for(auto &t : internalSnappy::decompressInputs) t->join();
        internalSnappy::decompressFunc->join();

        internalSnappy::decompressFunc.reset(nullptr);
        internalSnappy::decompressInputs.resize(0);
        internalSnappy::decompressInputCur=internalSnappy::decompressInputIdx=0;
    }

    return size;
}

// uint64_t snappyCompressMM(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     uint32_t outIdx=internalSnappy::writeHeader(out);
//     uint64_t enbytes=snappyCompressEngineMM(in, out+outIdx, inputSize);
//     outIdx+=enbytes;
//     return outIdx;
// }

// uint64_t snappyCompressStream(uint8_t* in, uint8_t* out, uint32_t inputSize){
//     uint32_t outIdx=internalSnappy::writeHeader(out);
//     uint64_t enbytes=snappyCompressEngineStream(in, out+outIdx, inputSize);
//     outIdx+=enbytes;
//     return outIdx;
// }

// uint64_t snappyDecompressMM(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     uint8_t headerBytes = internalSnappy::readHeader(in);
//     uint64_t debytes = snappyDecompressEngineMM(in+headerBytes, out, inputSize-headerBytes);
//     return debytes;
// }

// uint64_t snappyDecompressStream(uint8_t* in, uint8_t* out, uint32_t inputSize){
//     uint64_t debytes = snappyDecompressEngineStream(in, out, inputSize);
//     return debytes;
// }

// uint64_t snappyCompress(uint8_t* in, uint8_t* out, uint64_t inputSize, bool stream){
//     std::cout<<"Before Snappy Compress: "<<std::endl;
//     hexdump(in, inputSize);

//     uint64_t enbytes=stream ? snappyCompressStream(in, out, inputSize) : snappyCompressMM(in, out, inputSize);

//     std::cout<<"After Snappy Compress: "<<std::endl;
//     hexdump(out, enbytes);

//     return enbytes;
// }

// uint64_t snappyDecompress(uint8_t* in, uint8_t* out, uint64_t inputSize, bool stream){
//     std::cout<<"Before Snappy Decompress: "<<std::endl;
//     hexdump(in, inputSize);

//     uint64_t debytes = stream ? snappyDecompressStream(in, out, inputSize) : snappyDecompressMM(in, out, inputSize);

//     std::cout<<"After Snappy Decompress: "<<std::endl;
//     hexdump(out, debytes);

//     return debytes;
// }
} //dataCompression