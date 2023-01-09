#include "Lz4.hpp"
#include <bitset>

namespace dataCompression{
namespace internalLz4{
std::mutex compressInputMtx, compressOutputMtx, decompressInputMtx, decompressOutputMtx;
std::condition_variable compressInputCV, compressOutputCV, decompressInputCV, decompressOutputCV;
std::vector<std::unique_ptr<std::thread>> compressInputs, compressOutputs, decompressInputs, decompressOutputs;
std::unique_ptr<std::thread> compressFunc, decompressFunc;
uint32_t compressInputCur, compressOutputCur, decompressInputCur, decompressOutputCur;
uint32_t compressInputIdx, compressOutputIdx, decompressInputIdx, decompressOutputIdx;
uint32_t checksum;

ThreadSafeFIFO<uint8_t> compressQueueInput, compressQueueOutput;
ThreadSafeFIFO<uint8_t> decompressQueueInput, decompressQueueOutput;

// Maximum host buffer used to operate per kernel invocation
const auto HOST_BUFFER_SIZE = (32 * 1024 * 1024);

// Default block size
const auto BLOCK_SIZE_IN_KB = 64;

// Maximum number of blocks based on host buffer size
const auto MAX_NUMBER_BLOCKS = (HOST_BUFFER_SIZE / (BLOCK_SIZE_IN_KB * 1024));

void lz4CompressionEngine(){
    uint32_t hostBufferSize = HOST_BUFFER_SIZE;
    uint32_t maxNumBlocks = (hostBufferSize) / (BLOCK_SIZE_IN_KB * 1024);
    std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(hostBufferSize);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(hostBufferSize);
    std::vector<uint32_t, aligned_allocator<uint32_t>> blockSizeBufferHost(maxNumBlocks);
    std::vector<uint32_t, aligned_allocator<uint32_t>> compressSizeBufferHost(maxNumBlocks);

    uint32_t block_size_in_kb = BLOCK_SIZE_IN_KB;
    uint32_t block_size_in_bytes = block_size_in_kb * 1024;

    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
    KernelPointer lz4CompressKernel(Application::getProgram<Lib::LZ4>(), "xilLz4CompressMM:{xilLz4CompressMM_1}");
    bool last;

    do{
        uint32_t chunkSize=compressQueueInput.pop(inBufferHost.data(), hostBufferSize, last);
        std::cout<<"get input ["<<chunkSize<<" Bytes]"<<std::endl;
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
                compressQueueOutput.push(&compressed_size, 4, false);

                if(bIdx<numBlocks-1) compressQueueOutput.push(&(outBufferHost.data()[bIdx * block_size_in_bytes]), compressed_size, false);
                else compressQueueOutput.push(&(outBufferHost.data()[bIdx * block_size_in_bytes]), compressed_size, last);
                std::cout<<"compressed block size: "<<compressed_size<<std::endl;
                // hexdump(&(outBufferHost.data()[bIdx * block_size_in_bytes]), compressed_size);
            } else {
                uint8_t temp = 0;
                temp = blockSize;
                compressQueueOutput.push(temp, false);
                temp = blockSize >> 8;
                compressQueueOutput.push(temp, false);
                compressQueueOutput.push(0, false);
                compressQueueOutput.push(128, false);

                if(bIdx<numBlocks-1) compressQueueOutput.push(inBufferHost.data()+idx, blockSize, false);
                else compressQueueOutput.push(inBufferHost.data()+idx, blockSize, last);
                std::cout<<"compressed block size: "<<blockSize<<std::endl;
                // hexdump(inBufferHost.data()+idx, blockSize);
            }
        }
    }while(!last);
}

void lz4DecompressionEngine(){
    uint64_t hostBufferSize = internalLz4::HOST_BUFFER_SIZE;
    std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(hostBufferSize);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(hostBufferSize);

    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
    KernelPointer decompress_kernel_lz4(Application::getProgram<Lib::LZ4>(), "xilLz4DecompressMM:{xilLz4DecompressMM_1}");

    bool last;
    
    do{ // per frames
        decompressQueueInput.pop(inBufferHost.data(), 4, last);
        std::cout<<std::hex<<"magic number: "<<*(uint32_t*)inBufferHost.data()<<std::endl;
        if(*(uint32_t*)inBufferHost.data()==0x184D2204){
            //General Structure of LZ4 Frame format
            std::cout<<"General Structure of LZ4 Frame format"<<std::endl;

            //FLG
            decompressQueueInput.pop(inBufferHost.data(), 1, last);
            std::cout<<std::bitset<8>(inBufferHost[0])<<std::endl;
            uint8_t dictID=inBufferHost[0]&0x1;
            std::cout<<"dictID: "<<(uint32_t)dictID<<std::endl;
            if(dictID){
                std::cerr<<"Cannot decompress where dictID is set!"<<std::endl;
                exit(EXIT_FAILURE);
            }
            inBufferHost[0]>>=2;

            uint8_t contentChecksumFlg=inBufferHost[0]&0x1;
            std::cout<<"contentChecksumFlg: "<<(uint32_t)contentChecksumFlg<<std::endl;
            inBufferHost[0]>>=1;

            uint8_t contentSizeFlg=inBufferHost[0]&0x1;
            std::cout<<"contentSizeFlg: "<<(uint32_t)contentSizeFlg<<std::endl;
            inBufferHost[0]>>=1;

            uint8_t blockChecksumFlg=inBufferHost[0]&0x1;
            std::cout<<"blockChecksumFlg: "<<(uint32_t)blockChecksumFlg<<std::endl;
            inBufferHost[0]>>=1;

            uint8_t blockIndep=inBufferHost[0]&0x1;
            std::cout<<"blockIndep: "<<(uint32_t)blockIndep<<std::endl;
            if(!blockIndep){
                std::cerr<<"Cannot decompress where block independence is not set!"<<std::endl;
                exit(EXIT_FAILURE);
            }
            inBufferHost[0]>>=1;

            if(inBufferHost[0]!=0b01){
                std::cout<<"version: "<<(uint32_t)inBufferHost[0]<<std::endl;
                std::cerr<<"version incorrect!"<<std::endl;
                exit(EXIT_FAILURE);
            }

            //BD
            decompressQueueInput.pop(inBufferHost.data(), 1, last);
            std::cout<<std::bitset<8>(inBufferHost[0])<<std::endl;
            inBufferHost[0]>>=4;
            uint32_t blockMaxSizeFlg=inBufferHost[0]&0b111;
            uint32_t block_size_in_bytes;

            switch(blockMaxSizeFlg){
                case 4: block_size_in_bytes=64*1024; break;
                case 5: block_size_in_bytes=256*1024; break;
                case 6: block_size_in_bytes=1*1024*1024; break;
                case 7: block_size_in_bytes=4*1024*1024; break;
                default: std::cerr<<"Block Max Size FLG invalid!"<<std::endl; exit(EXIT_FAILURE);
            }
            std::cout<<"block_size_in_bytes: "<<block_size_in_bytes<<std::endl;

            uint32_t maxNumBlocks=hostBufferSize/block_size_in_bytes;
            std::vector<uint32_t, aligned_allocator<uint32_t>> decompressSizeBufferHost(maxNumBlocks);
            std::vector<uint32_t, aligned_allocator<uint32_t>> compressSizeBufferHost(maxNumBlocks);

            // Device buffer allocation
            BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, hostBufferSize, inBufferHost.data());
            BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, hostBufferSize, outBufferHost.data());
            BufferPointer decompressdSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t)*maxNumBlocks, decompressSizeBufferHost.data());
            BufferPointer compressdSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t)*maxNumBlocks, compressSizeBufferHost.data());
            
            //content size
            if(contentSizeFlg) decompressQueueInput.pop(inBufferHost.data(), 4, last);

            //HC
            decompressQueueInput.pop(inBufferHost.data(), 1, last);

            //data blocks
            decompressQueueInput.pop(inBufferHost.data(), 4, last);

            uint32_t bufferTotalBlock=0;

            auto executeWrite=[&]{
                std::cout<<"start for decompressing blocks"<<std::endl;
                decompress_kernel_lz4->setArg(0, *inputBuffer);
                decompress_kernel_lz4->setArg(1, *outputBuffer);
                decompress_kernel_lz4->setArg(2, *decompressdSizeBuffer);
                decompress_kernel_lz4->setArg(3, *compressdSizeBuffer);
                decompress_kernel_lz4->setArg(4, block_size_in_bytes/1024);
                decompress_kernel_lz4->setArg(5, bufferTotalBlock);

                queue->enqueueMigrateMemObjects({*inputBuffer, *compressdSizeBuffer}, 0);
                queue->finish();

                queue->enqueueTask(*decompress_kernel_lz4);
                queue->finish();

                queue->enqueueMigrateMemObjects({*decompressdSizeBuffer, *outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
                queue->finish();

                for(uint32_t i=0;i<bufferTotalBlock;i++){
                    std::cout<<i<<" "<<decompressSizeBufferHost[i]<<std::endl;
                }

                for(uint32_t blockIdx=0, decompressedSize=0;blockIdx<bufferTotalBlock;blockIdx++){
                    uint32_t blockSize=decompressSizeBufferHost[blockIdx];
                    decompressQueueOutput.push(outBufferHost.data()+decompressedSize, blockSize, false);
                    std::cout<<"decompressed a block"<<std::endl;
                    // hexdump(outBufferHost.data()+decompressedSize, blockSize);
                    decompressedSize+=blockSize;
                }

                bufferTotalBlock=0;
            };

            while(*(uint32_t*)(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes)!=0x0){
                uint32_t compressedSize=*(uint32_t*)(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes);
                if(compressedSize>>31) compressedSize=compressedSize&0x7FFFFFFF;
                std::cout<<"trying to read a block, size: "<<std::dec<<compressedSize<<std::endl;

                compressSizeBufferHost[bufferTotalBlock]=compressedSize;
                decompressQueueInput.pop(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes, compressedSize, last);
                // hexdump(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes, compressedSize);
                bufferTotalBlock++;

                //这里必须要减一，不然decompressedSize数据前面全是0，就最后一个是65536
                if(bufferTotalBlock==maxNumBlocks-1) executeWrite();
                
                uint32_t _;
                if(blockChecksumFlg) decompressQueueInput.pop(&_, 4, last);
                
                std::cout<<"finished reading a block"<<std::endl;
                //next data block size
                decompressQueueInput.pop(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes, 4, last);
            }

            if(bufferTotalBlock) executeWrite();

            if(contentChecksumFlg) decompressQueueInput.pop(inBufferHost.data(), 4, last);
        }else if(0x184D2A50<=*(uint32_t*)inBufferHost.data() && *(uint32_t*)inBufferHost.data()<=0x184D2A5F){
            uint32_t size;
            decompressQueueInput.pop(&size, 4, last);

            while(size--) decompressQueueInput.pop();
        }

        //结束标识出现在输出之后，无法提前得知，所以需要多写一个，会在读出的时候被去掉
        if(last){
            decompressQueueOutput.push(0, true);
            std::cout<<"finish decompressing"<<std::endl;
        }   

        std::cout<<"frame end"<<std::endl;
    }while(!last);
}
}

uint8_t writeLz4Header(uint8_t* out, uint64_t inputSize){
    uint8_t fileIdx = 0;
    (out[fileIdx++]) = xf::compression::MAGIC_BYTE_1;
    (out[fileIdx++]) = xf::compression::MAGIC_BYTE_2;
    (out[fileIdx++]) = xf::compression::MAGIC_BYTE_3;
    (out[fileIdx++]) = xf::compression::MAGIC_BYTE_4;

    // FLG & BD bytes
    // --no-frame-crc flow
    // --content-size
    (out[fileIdx++]) = xf::compression::FLG_BYTE;

    uint64_t block_size_header = 0;
    // Default value 64K
    switch (internalLz4::BLOCK_SIZE_IN_KB) {
        case 64:
            (out[fileIdx++]) = xf::compression::BSIZE_STD_64KB;
            block_size_header = xf::compression::BSIZE_STD_64KB;
            break;
        case 256:
            (out[fileIdx++]) = xf::compression::BSIZE_STD_256KB;
            block_size_header = xf::compression::BSIZE_STD_256KB;
            break;
        case 1024:
            (out[fileIdx++]) = xf::compression::BSIZE_STD_1024KB;
            block_size_header = xf::compression::BSIZE_STD_1024KB;
            break;
        case 4096:
            (out[fileIdx++]) = xf::compression::BSIZE_STD_4096KB;
            block_size_header = xf::compression::BSIZE_STD_4096KB;
            break;
        default:
            std::cout << "Invalid Block Size" << std::endl;
            break;
    }

    uint8_t temp_buff[10] = {xf::compression::FLG_BYTE,
                             (uint8_t)block_size_header,
                             (uint8_t)inputSize,
                             (uint8_t)(inputSize >> 8),
                             (uint8_t)(inputSize >> 16),
                             (uint8_t)(inputSize >> 24),
                             (uint8_t)(inputSize >> 32),
                             (uint8_t)(inputSize >> 40),
                             (uint8_t)(inputSize >> 48),
                             (uint8_t)(inputSize >> 56)};

    // xxhash is used to calculate hash value
    uint32_t xxh = XXH32(temp_buff, 2, 0);

    //  Header CRC
    out[fileIdx++] = (uint8_t)(xxh >> 8);

    return fileIdx;
}

uint8_t writeLz4Footer(uint8_t* out){
    //temporarily calculate the checksum like this
    // if(inputSize>internalLz4::BLOCK_SIZE_IN_KB) inputSize=internalLz4::BLOCK_SIZE_IN_KB;

    uint8_t fileIdx = 0;
    uint32_t* zero_ptr = 0;
    memcpy(out, &zero_ptr, 4);
    fileIdx += 4;

    // xxhash is used to calculate content checksum value
    // uint32_t xxh = XXH32(in, inputSize, 0);
    // memcpy(out+fileIdx, &xxh, 4);
    // fileIdx += 4;
    return fileIdx;
}

uint8_t readLz4Header(uint8_t* in){
    uint8_t fileIdx = 0;
    // Read magic header 4 bytes
    char c = 0;
    int magic_hdr[] = {xf::compression::MAGIC_BYTE_1, xf::compression::MAGIC_BYTE_2, xf::compression::MAGIC_BYTE_3, xf::compression::MAGIC_BYTE_4};
    for (uint32_t i = 0; i < xf::compression::MAGIC_HEADER_SIZE; i++) {
        // inFile.get(v);
        c = in[fileIdx++];
        if (int(c) == magic_hdr[i])
            continue;
        else {
            std::cerr << "Problem with magic header " << c << " " << i << std::endl;
            exit(1);
        }
    }

    // FLG Byte
    c = in[fileIdx++];

    // Check if block size is 64 KB
    c = in[fileIdx++];

    // switch (c) {
    //     case xf::compression::BSIZE_STD_64KB:
    //         m_BlockSizeInKb = 64;
    //         break;
    //     case xf::compression::BSIZE_STD_256KB:
    //         m_BlockSizeInKb = 256;
    //         break;
    //     case xf::compression::BSIZE_STD_1024KB:
    //         m_BlockSizeInKb = 1024;
    //         break;
    //     case xf::compression::BSIZE_STD_4096KB:
    //         m_BlockSizeInKb = 4096;
    //         break;
    //     default:
    //         std::cout << "Invalid Block Size" << std::endl;
    //         break;
    // }

    if (xf::compression::FLG_BYTE == 104) {
        // Original size
        uint64_t original_size = 0;

        memcpy(&original_size, &in[fileIdx], 8);

        // file size(8)
        fileIdx += 8;
    }

    // Header Checksum
    c = in[fileIdx++];

    // m_HostBufferSize = (m_BlockSizeInKb * 1024) * MAX_NUMBER_BLOCKS;

    return fileIdx;
}

void lz4CompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last){
    if(internalLz4::compressFunc==nullptr){
        internalLz4::compressFunc.reset(nullptr);
        internalLz4::compressFunc=std::make_unique<std::thread>(&internalLz4::lz4CompressionEngine);

        internalLz4::compressInputs.resize(0);
        internalLz4::compressInputCur=internalLz4::compressInputIdx=0;
    }

    uint32_t compressInputIdx=internalLz4::compressInputIdx++;
    internalLz4::compressInputs.push_back(std::make_unique<std::thread>([in, inputSize, last, compressInputIdx]{
        std::cout<<"write thread "<<compressInputIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalLz4::compressInputMtx);
        internalLz4::compressInputCV.wait(lck, [compressInputIdx] {return internalLz4::compressInputCur==compressInputIdx;});

        std::cout<<"write thread "<<compressInputIdx<<" start to write"<<std::endl;
        internalLz4::compressQueueInput.push(in, inputSize, last);

        internalLz4::compressInputCur++;
        internalLz4::compressInputCV.notify_all();
        std::cout<<"write thread "<<compressInputIdx<<" finished"<<std::endl;
    }));
}

void lz4CompressionInput(uint8_t *in, uint32_t inputSize, bool last){
    if(internalLz4::compressFunc==nullptr){
        internalLz4::compressFunc.reset(nullptr);
        internalLz4::compressFunc=std::make_unique<std::thread>(&internalLz4::lz4CompressionEngine);
    }

    internalLz4::compressQueueInput.push(in, inputSize, last);
}

uint32_t lz4CompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size;

    uint32_t compressOutputIdx=internalLz4::compressOutputIdx++;
    internalLz4::compressOutputs.push_back(std::make_unique<std::thread>([out, outputSize, &last, compressOutputIdx, &size]{
        std::cout<<"write thread "<<compressOutputIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalLz4::compressOutputMtx);
        internalLz4::compressOutputCV.wait(lck, [compressOutputIdx] {return internalLz4::compressOutputCur==compressOutputIdx;});

        std::cout<<"write thread "<<compressOutputIdx<<" start to write"<<std::endl;
        size=internalLz4::compressQueueOutput.pop(out, outputSize, last);

        internalLz4::compressOutputCur++;
        internalLz4::compressOutputCV.notify_all();
        std::cout<<"write thread "<<compressOutputIdx<<" finished"<<std::endl;
    }));

    if(last){
        for(auto &t : internalLz4::compressInputs) t->join();
        for(auto &t : internalLz4::compressOutputs) t->join();
        internalLz4::compressFunc->join();

        internalLz4::compressFunc.reset(nullptr);
        internalLz4::compressInputs.resize(0);
        internalLz4::compressOutputs.resize(0);
        internalLz4::compressInputCur=internalLz4::compressInputIdx=0;
        internalLz4::compressOutputCur=internalLz4::compressOutputIdx=0;
    }

    return size;
}

uint32_t lz4CompressionOutput(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size=internalLz4::compressQueueOutput.pop(out, outputSize, last);

    if(last){
        internalLz4::compressFunc->join();
        internalLz4::compressFunc.reset(nullptr);
    }

    return size;
}

void lz4DecompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last){
    if(internalLz4::decompressFunc==nullptr){
        internalLz4::decompressFunc.reset(nullptr);
        internalLz4::decompressFunc=std::make_unique<std::thread>(&internalLz4::lz4DecompressionEngine);

        internalLz4::decompressInputs.resize(0);
        internalLz4::decompressInputCur=internalLz4::decompressInputIdx=0;
    }

    uint32_t decompressInputIdx=internalLz4::decompressInputIdx++;
    internalLz4::decompressInputs.push_back(std::make_unique<std::thread>([in, inputSize, last, decompressInputIdx]{
        std::cout<<"write thread "<<decompressInputIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalLz4::decompressInputMtx);
        internalLz4::decompressInputCV.wait(lck, [decompressInputIdx] {return internalLz4::decompressInputCur==decompressInputIdx;});

        std::cout<<"write thread "<<decompressInputIdx<<" start to write"<<std::endl;
        internalLz4::decompressQueueInput.push(in, inputSize, last);

        internalLz4::decompressInputCur++;
        internalLz4::decompressInputCV.notify_all();
        std::cout<<"write thread "<<decompressInputIdx<<" finished"<<std::endl;
    }));
}

void lz4DecompressionInput(uint8_t *in, uint32_t inputSize, bool last){
    if(internalLz4::decompressFunc==nullptr){
        internalLz4::decompressFunc.reset(nullptr);
        internalLz4::decompressFunc=std::make_unique<std::thread>(&internalLz4::lz4DecompressionEngine);
    }

    internalLz4::decompressQueueInput.push(in, inputSize, last);
}

uint32_t lz4DecompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size;

    uint32_t decompressOutputIdx=internalLz4::decompressOutputIdx++;
    internalLz4::decompressOutputs.push_back(std::make_unique<std::thread>([out, outputSize, &last, decompressOutputIdx, &size]{
        std::cout<<"write thread "<<decompressOutputIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalLz4::decompressOutputMtx);
        internalLz4::decompressOutputCV.wait(lck, [decompressOutputIdx] {return internalLz4::decompressOutputCur==decompressOutputIdx;});

        std::cout<<"write thread "<<decompressOutputIdx<<" start to write"<<std::endl;
        size=internalLz4::decompressQueueOutput.pop(out, outputSize, last);

        internalLz4::decompressOutputCur++;
        internalLz4::decompressOutputCV.notify_all();
        std::cout<<"write thread "<<decompressOutputIdx<<" finished"<<std::endl;
    }));

    if(last){
        for(auto &t : internalLz4::decompressInputs) t->join();
        for(auto &t : internalLz4::decompressOutputs) t->join();
        internalLz4::decompressFunc->join();

        internalLz4::decompressFunc.reset(nullptr);
        internalLz4::decompressInputs.resize(0);
        internalLz4::decompressOutputs.resize(0);
        internalLz4::decompressInputCur=internalLz4::decompressInputIdx=0;
        internalLz4::decompressOutputCur=internalLz4::decompressOutputIdx=0;
    }

    if(last) size--;
    return size;
}

uint32_t lz4DecompressionOutput(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size=internalLz4::decompressQueueOutput.pop(out, outputSize, last);

    if(last){
        for(auto &t : internalLz4::decompressInputs) t->join();
        internalLz4::decompressFunc->join();

        internalLz4::decompressFunc.reset(nullptr);
        internalLz4::decompressInputs.resize(0);
        internalLz4::decompressInputCur=internalLz4::decompressInputIdx=0;
    }

    if(last) size--;
    return size;
}

// uint64_t lz4CompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(internalLz4::BLOCK_SIZE_IN_KB * 1024);
//     std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(internalLz4::BLOCK_SIZE_IN_KB * 1024);
//     std::vector<uint32_t, aligned_allocator<uint32_t>> compressSizeBufferHost(1);

//     uint32_t hostBufferSize = internalLz4::BLOCK_SIZE_IN_KB * 1024;
//     uint32_t total_block_count = (inputSize - 1) / hostBufferSize + 1;

//     uint64_t outIdx = 0;
//     BufferPointer compressdSizeBuffer(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), compressSizeBufferHost.data());
//     BufferPointer inputBuffer(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, hostBufferSize, inBufferHost.data());
//     BufferPointer outputBuffer(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, hostBufferSize, outBufferHost.data());

//     CommandQueuePointer queue(Application::getContext<Lib::LZ4>(), Application::getDevice<Lib::LZ4>(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
//     KernelPointer compress_data_mover_kernel(Application::getProgram<Lib::LZ4>(), "xilCompressDatamover:{xilCompressDatamover_2}");
//     KernelPointer lz4CompressKernel(Application::getProgram<Lib::LZ4>(), "xilLz4CompressStream:{xilLz4CompressStream_1}");

//     // sequentially copy block sized buffers to kernel and wait for them to finish before enqueueing
//     for (uint32_t blkIndx = 0, bufIndx = 0; blkIndx < total_block_count; blkIndx++, bufIndx += hostBufferSize) {
//         // current block input size
//         uint32_t c_input_size = hostBufferSize;
//         if (blkIndx == total_block_count - 1) c_input_size = inputSize - bufIndx;

//         // copy input to input buffer
//         std::memcpy(inBufferHost.data(), in + bufIndx, c_input_size);

//         // set kernel args
//         uint32_t narg = 0;
//         compress_data_mover_kernel->setArg(narg++, *inputBuffer);
//         compress_data_mover_kernel->setArg(narg++, *outputBuffer);
//         compress_data_mover_kernel->setArg(narg++, *compressdSizeBuffer);
//         compress_data_mover_kernel->setArg(narg, c_input_size);

//         lz4CompressKernel->setArg(2, c_input_size);
//         // Migrate Memory - Map host to device buffers
//         queue->enqueueMigrateMemObjects({*(inputBuffer)}, 0);
//         queue->finish();

//         // enqueue the kernels and wait for them to finish
//         queue->enqueueTask(*compress_data_mover_kernel);
//         queue->enqueueTask(*lz4CompressKernel);
//         queue->finish();

//         // Setup output buffer vectors
//         std::vector<cl::Memory> outBufVec;
//         outBufVec.push_back(*outputBuffer);
//         outBufVec.push_back(*compressdSizeBuffer);

//         // Migrate memory - Map device to host buffers
//         queue->enqueueMigrateMemObjects(outBufVec, CL_MIGRATE_MEM_OBJECT_HOST);
//         queue->finish();
//         // read the data to output buffer
//         // copy the compressed data to out pointer
//         uint32_t compressedSize = compressSizeBufferHost.data()[0];

//         if (c_input_size > compressedSize) {
//             // copy the compressed data
//             std::memcpy(out + outIdx, &compressedSize, 4);
//             outIdx += 4;
//             std::memcpy(out + outIdx, outBufferHost.data(), compressedSize);
//             outIdx += compressedSize;
//         } else {
//             // copy the original data, since no compression
//             if (c_input_size == hostBufferSize) {
//                 out[outIdx++] = 0;
//                 out[outIdx++] = 0;
//                 out[outIdx++] = internalLz4::get_bsize(c_input_size);
//                 out[outIdx++] = xf::compression::NO_COMPRESS_BIT;
//             } else {
//                 uint8_t tmp = c_input_size;
//                 out[outIdx++] = tmp;
//                 tmp = c_input_size >> 8;
//                 out[outIdx++] = tmp;
//                 tmp = c_input_size >> 16;
//                 out[outIdx++] = tmp;
//                 out[outIdx++] = xf::compression::NO_COMPRESS_BIT;
//             }
//             std::memcpy(out + outIdx, in + (hostBufferSize * blkIndx), c_input_size);
//             outIdx += c_input_size;
//         }
//     }

//     return outIdx;
// }

// uint64_t lz4DecompressEngineMM(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     uint64_t hostBufferSize = internalLz4::HOST_BUFFER_SIZE*2;
//     uint32_t maxNumBlocks = (hostBufferSize) / (internalLz4::BLOCK_SIZE_IN_KB * 1024);
//     std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(hostBufferSize);
//     std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(hostBufferSize);
//     std::vector<uint32_t, aligned_allocator<uint32_t>> decompressSizeBufferHost(maxNumBlocks);
//     std::vector<uint32_t, aligned_allocator<uint32_t>> compressSizeBufferHost(maxNumBlocks);

//     uint32_t block_size_in_bytes = internalLz4::BLOCK_SIZE_IN_KB * 1024;

//     uint64_t inIdx = 0;
//     uint64_t total_decomression_size = 0;

//     uint64_t chunkSize;
//     uint64_t output_idx = 0;

//     // To handle uncompressed blocks
//     bool compressBlk = false;

//     // Device buffer allocation
//     BufferPointer inputBuffer(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, hostBufferSize, inBufferHost.data());
//     BufferPointer outputBuffer(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, hostBufferSize*2, outBufferHost.data());
//     BufferPointer decompressdSizeBuffer(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t) * maxNumBlocks, decompressSizeBufferHost.data());
//     BufferPointer compressdSizeBuffer(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t) * maxNumBlocks, compressSizeBufferHost.data());

//     CommandQueuePointer queue(Application::getContext<Lib::LZ4>(), Application::getDevice<Lib::LZ4>(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
//     KernelPointer decompress_kernel_lz4(Application::getProgram<Lib::LZ4>(), "xilLz4DecompressMM:{xilLz4DecompressMM_1}");

//     for (; inIdx < inputSize;) {
//         uint64_t chunk_size = hostBufferSize;

//         // Figure out the chunk size for each compute unit
//         chunkSize = 0;
//         if (inIdx + (chunk_size) > inputSize) {
//             chunkSize = inputSize - (inIdx);
//         } else {
//             chunkSize = chunk_size;
//         }

//         uint32_t numBlocks;
//         uint32_t bufblocks;
//         uint64_t total_size;
//         uint64_t buf_size;
//         uint32_t blockSize = 0;
//         uint32_t compressed_size = 0;

//         numBlocks = 0;
//         buf_size = 0;
//         bufblocks = 0;
//         total_size = 0;
//         for (uint64_t cIdx = 0; cIdx < chunkSize; numBlocks++, total_size += blockSize) {
//             blockSize = block_size_in_bytes;
//             std::memcpy(&compressed_size, &in[inIdx], 4);
//             inIdx += 4;
//             cIdx += 4;

//             uint32_t tmp = compressed_size;
//             tmp >>= 24;

//             if (tmp == 128) {
//                 uint8_t b1 = compressed_size;
//                 uint8_t b2 = compressed_size >> 8;
//                 uint8_t b3 = compressed_size >> 16;
//                 // uint8_t b4 = compressed_size >> 24;

//                 if (b3 == 1) {
//                     compressed_size = block_size_in_bytes;
//                 } else {
//                     uint16_t size = 0;
//                     size = b2;
//                     size <<= 8;
//                     uint16_t temp = b1;
//                     size |= temp;
//                     compressed_size = size;
//                 }
//             }
//             // Fill original block size and compressed size
//             compressSizeBufferHost.data()[bufblocks] = compressed_size;
//             std::memcpy(&(inBufferHost.data()[buf_size]), &in[inIdx], compressed_size);
//             inIdx += compressed_size;
//             cIdx += compressed_size;
//             buf_size += blockSize;
//             bufblocks++;
//             compressBlk = true;
//         }

//         if (numBlocks == 1 && compressed_size == blockSize){
//             std::cout<<"test"<<std::endl;
//             break;
//         }
//         if (compressBlk) {
//             // Set kernel arguments
//             uint32_t narg = 0;
//             decompress_kernel_lz4->setArg(narg++, *(inputBuffer));
//             decompress_kernel_lz4->setArg(narg++, *(outputBuffer));
//             decompress_kernel_lz4->setArg(narg++, *(decompressdSizeBuffer));
//             decompress_kernel_lz4->setArg(narg++, *(compressdSizeBuffer));
//             decompress_kernel_lz4->setArg(narg++, (uint32_t)internalLz4::BLOCK_SIZE_IN_KB);
//             decompress_kernel_lz4->setArg(narg++, bufblocks);

//             std::cout<<"bufblocks: "<<bufblocks<<std::endl;


//             std::vector<cl::Memory> inBufVec;
//             inBufVec.push_back(*(inputBuffer));
//             inBufVec.push_back(*(compressdSizeBuffer));
//             // Migrate memory - Map host to device buffers
//             queue->enqueueMigrateMemObjects(inBufVec, 0 /* 0 means from host*/);
//             queue->finish();

//             // Kernel invocation
//             queue->enqueueTask(*decompress_kernel_lz4);
//             queue->finish();

//             std::vector<cl::Memory> outBufVec;
//             outBufVec.push_back(*(decompressdSizeBuffer));
//             outBufVec.push_back(*(outputBuffer));
//             // Migrate memory - Map device to host buffers
//             queue->enqueueMigrateMemObjects(outBufVec, CL_MIGRATE_MEM_OBJECT_HOST);
//             queue->finish();
//         }
//         uint32_t bufIdx = 0;
//         for (uint32_t bIdx = 0; bIdx < numBlocks; bIdx++) {
//             uint32_t blockSize = decompressSizeBufferHost.data()[bIdx];
            
//             std::memcpy(&out[output_idx], &outBufferHost.data()[bufIdx], blockSize);
//             output_idx += blockSize;
//             bufIdx += blockSize;
//             total_decomression_size += blockSize;
//         }

//     }

//     return total_decomression_size;
// }

// uint64_t lz4DecompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     std::vector<uint32_t, aligned_allocator<uint32_t> > decompressSize;
//     uint32_t outputSize = inputSize*2;
    
//     // Index calculation
//     std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(inputSize);
//     std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(outputSize);
//     std::vector<uint32_t, aligned_allocator<uint32_t>> h_buf_decompressSize(sizeof(uint32_t));

//     std::memcpy(inBufferHost.data(), in, inputSize);

//     // Device buffer allocation
//     BufferPointer inputBuffer(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, inputSize, inBufferHost.data());
//     BufferPointer outputBuffer(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, outputSize, outBufferHost.data());
//     BufferPointer bufferOutputSize(Application::getContext<Lib::LZ4>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), h_buf_decompressSize.data());

//     uint32_t inputSize_32t = uint32_t(inputSize);

//     KernelPointer decompress_data_mover_kernel(Application::getProgram<Lib::LZ4>(), "xilDecompressDatamover:{xilDecompressDatamover_1}");
//     KernelPointer decompress_kernel_lz4(Application::getProgram<Lib::LZ4>(), "xilLz4DecompressStream:{xilLz4DecompressStream_1}");
//     CommandQueuePointer queue(Application::getContext<Lib::LZ4>(), Application::getDevice<Lib::LZ4>(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

//     // set kernel arguments
//     int narg = 0;
//     decompress_data_mover_kernel->setArg(narg++, *(inputBuffer));
//     decompress_data_mover_kernel->setArg(narg++, *(outputBuffer));
//     decompress_data_mover_kernel->setArg(narg++, inputSize_32t);
//     decompress_data_mover_kernel->setArg(narg, *(bufferOutputSize));

//     decompress_kernel_lz4->setArg(3, inputSize_32t);

//     // Migrate Memory - Map host to device buffers
//     queue->enqueueMigrateMemObjects({*inputBuffer, *bufferOutputSize, *outputBuffer}, 0);
//     queue->finish();

//     // enqueue the kernels and wait for them to finish
//     queue->enqueueTask(*decompress_data_mover_kernel);
//     queue->enqueueTask(*decompress_kernel_lz4);
//     queue->finish();

//     // Migrate memory - Map device to host buffers
//     queue->enqueueMigrateMemObjects({*outputBuffer, *bufferOutputSize}, CL_MIGRATE_MEM_OBJECT_HOST);
//     queue->finish();

//     uint32_t uncompressedSize = h_buf_decompressSize[0];
//     std::memcpy(out, outBufferHost.data(), uncompressedSize);

//     return uncompressedSize;
// }

// uint64_t lz4CompressMM(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     uint64_t outIdx=0;

//     // LZ4 header
//     outIdx+=writeLz4Header(out+outIdx, inputSize);

//     uint64_t enbytes=lz4CompressEngineMM(in, out+outIdx, inputSize);
//     outIdx+=enbytes;

//     outIdx+=writeLz4Footer(in, out+outIdx, inputSize);
//     return outIdx;
// }

// uint64_t lz4CompressStream(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     uint64_t outIdx=0;

//     // LZ4 header
//     outIdx += writeLz4Header(out+outIdx, inputSize);

//     uint64_t enbytes = lz4CompressEngineStream(in, out+outIdx, inputSize);
//     outIdx+=enbytes;

//     outIdx+=writeLz4Footer(in, out+outIdx, inputSize);
//     return outIdx;
// }

// uint64_t lz4DecompressMM(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     in += readLz4Header(in);
//     inputSize = inputSize - 15;
//     uint64_t debytes = lz4DecompressEngineMM(in, out, inputSize);
//     return debytes;
// }

// uint64_t lz4DecompressStream(uint8_t* in, uint8_t* out, uint64_t inputSize) {
//     uint64_t debytes = lz4DecompressEngineStream(in, out, inputSize);
//     return debytes;
// }

// uint64_t lz4Compress(uint8_t* in, uint8_t* out, uint64_t inputSize, bool stream){
//     uint64_t enbytes = stream ? lz4CompressStream(in, out, inputSize) : lz4CompressMM(in, out, inputSize);
//     return enbytes;
// }

// uint64_t lz4Decompress(uint8_t* in, uint8_t* out, uint64_t inputSize, bool stream){
//     uint64_t debytes = stream ? lz4DecompressStream(in, out, inputSize) : lz4DecompressMM(in, out, inputSize);
//     return debytes;
// }
}