#include "Zstd.hpp"

namespace dataCompression{
namespace internalZstd{
std::mutex compressMtx, decompressMtx;
std::condition_variable compressCV, decompressCV;
std::vector<std::unique_ptr<std::thread>> compressInputs, decompressInputs;
std::unique_ptr<std::thread> compressFunc, decompressFunc;
uint32_t compressCur, decompressCur;
uint32_t compressIdx, decompressIdx;
uint32_t checksum;

ThreadSafeFIFO<uint8_t> compressQueueInput, compressQueueOutput;
ThreadSafeFIFO<uint8_t> decompressQueueInput, decompressQueueOutput;

const uint64_t CHUNK_SIZE_IN_KB=16;
const uint64_t CHUNK_SIZE_IN_BYTE=CHUNK_SIZE_IN_KB*1024;
}

void zstdCompressEngineStream(){
    cl_int err;

    // host allocated aligned memory
    std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(internalZstd::CHUNK_SIZE_IN_BYTE);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(internalZstd::CHUNK_SIZE_IN_BYTE);
    std::vector<uint32_t, aligned_allocator<uint32_t>> outSizeBufferHost(1);

    BufferPointer inBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, internalZstd::CHUNK_SIZE_IN_BYTE, inBufferHost.data());
    BufferPointer outBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, internalZstd::CHUNK_SIZE_IN_BYTE, outBufferHost.data());
    BufferPointer outSizeBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), outSizeBufferHost.data());

    // set consistent kernel arguments
    KernelPointer cmp_dm_kernel(Application::getProgram<Lib::ZSTD>(), "xilZstdCompressDataMover:{xilZstdCompressDataMover_1}");
    cmp_dm_kernel->setArg(0, *inBuffer);
    cmp_dm_kernel->setArg(1, *outBuffer);
    cmp_dm_kernel->setArg(3, *outSizeBuffer);

    CommandQueuePointer queue(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);

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
        uint32_t chunkSize=internalZstd::compressQueueInput.pop(inBufferHost.data(), internalZstd::CHUNK_SIZE_IN_BYTE, last);
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
        internalZstd::compressQueueOutput.push(outBufferHost.data(), outSizeBufferHost[0], last);
        // auto compSize = outSizeBufferHost[0];
        // std::memcpy(out+outIdx, outBufferHost.data(), compSize);
        // enbytes += compSize;
        // outIdx += compSize;
    }while(!last);

    // return enbytes;
}

void zstdDecompressEngineStream(){
    CommandQueuePointer chunkWriterQueue(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);
    CommandQueuePointer chunkReaderQueue(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);

    KernelPointer chunkWriterKernel(Application::getProgram<Lib::ZSTD>(), "xilMM2S:{xilMM2S_1}");
    KernelPointer chunkReaderKernel(Application::getProgram<Lib::ZSTD>(), "xilS2MM:{xilS2MM_1}");

    //memory for compression
    std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(internalZstd::CHUNK_SIZE_IN_BYTE+4);
    BufferPointer inputBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, inputBufferHost.size(), inputBufferHost.data());

    //memory for decompression
    std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(internalZstd::CHUNK_SIZE_IN_BYTE);
    std::vector<uint32_t, aligned_allocator<uint32_t>> outputSizeBufferHost(1);
    std::vector<uint32_t, aligned_allocator<uint32_t>> statusBufferHost(1);

    BufferPointer outputBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint8_t)*outputBufferHost.size(), outputBufferHost.data());
    BufferPointer outputSizeBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), outputSizeBufferHost.data());
    BufferPointer statusBuffer(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), statusBufferHost.data());

    //kernel args fixed setting
    chunkWriterKernel->setArg(0, *inputBuffer);

    chunkReaderKernel->setArg(0, *outputBuffer);
    chunkReaderKernel->setArg(1, *outputSizeBuffer);
    chunkReaderKernel->setArg(2, *statusBuffer);
    chunkReaderKernel->setArg(3, (uint32_t)outputBufferHost.size());

    bool last, valid=false, first=true;
    uint32_t endBufferHost=0;
    std::vector<uint8_t> window;

    do{
        std::thread chunkWriter([&chunkWriterQueue, &chunkWriterKernel, &inputBufferHost, &inputBuffer, &last, &valid, &first, &endBufferHost, &window]{
            const uint32_t magicZstandardNumber=0xFD2FB528;
            const uint32_t magicSkippableNumberMin=0x184D2A50;
            const uint32_t magicSkippableNumberMax=0x184D2A5F;

            auto executeWrite=[&chunkWriterQueue, &chunkWriterKernel, &inputBufferHost, &inputBuffer](uint32_t size, bool last){
                std::cout<<"start writing a block of "<<size<<", last: "<<last<<std::endl;
                // hexdump(inputBufferHost.data(), size);

                chunkWriterQueue->enqueueMigrateMemObjects({*inputBuffer}, 0);
                chunkWriterQueue->finish();

                chunkWriterKernel->setArg(1, size);
                chunkWriterKernel->setArg(2, (uint32_t)last);

                chunkWriterQueue->enqueueTask(*chunkWriterKernel);
                chunkWriterQueue->finish();

                std::cout<<"write a chunk["<<size<<" Bytes], last = "<<last<<std::endl;
            };

            do{
                auto [curVal, curLast]=internalZstd::decompressQueueInput.pop();
                last=curLast;
                // std::cout<<std::hex<<"get a Byte: "<<(uint32_t)curVal<<" "<<curLast<<std::endl;

                if(window.size()<3){
                    window.push_back(curVal);
                    if(curLast){
                        std::cerr<<"File format error!"<<std::endl;
                        exit(EXIT_FAILURE);
                    }
                }else{
                    if(window.size()>3){
                        if(!first) inputBufferHost[endBufferHost++]=window[0];
                        window.erase(window.begin());
                    }
                    window.push_back(curVal);

                    //已经是最后一个byte了。即使window里面凑成了一个frame header，后续也不可能跟数据了，所以直接当作是数据
                    if(curLast){
                        last=curLast;
                        //这里即使endBufferHost>CHUNK_SIZE_IN_BYTE也可以继续塞，因为一开始就inputBufferHost就多开了4个byte的空间
                        for(auto v : window) inputBufferHost[endBufferHost++]=v;
                        executeWrite(endBufferHost, true);
                    }

                    // std::cout<<"Window: "<<std::hex<<*(uint32_t*)window.data()<<std::endl;
                    // for(int i=0;i<4;i++) std::cout<<std::hex<<window[i]<<" ";
                    // for(int i=0;i<4;i++) std::cout<<(char)window[i]<<" ";
                    // std::cout<<std::endl;
                    //判断window里面存的是不是某个Header
                    if((*(uint32_t*)window.data())==magicZstandardNumber || (magicSkippableNumberMin<=(*(uint32_t*)window.data()) && (*(uint32_t*)window.data())<=magicSkippableNumberMax)){
                        // std::cout<<"window is header"<<std::endl;
                        if(first) first=false;
                        else{
                            //如果不是第一个Header，那就要把之前的写掉
                            // std::cout<<"execute 168"<<std::endl;
                            executeWrite(endBufferHost, true);
                            endBufferHost=0;
                            break;
                        }
                    }

                    // if(*(uint32_t*)window.data()==magicZstandardNumber) valid=true;
                    // if(magicSkippableNumberMin<=*(uint32_t*)window.data() && *(uint32_t*)window.data()<=magicSkippableNumberMax) valid=false;

                    //这一步保证上面的endBufferHost的值域是[0, CHUNK_SIZE_IN_BYTE)
                    if(endBufferHost>=internalZstd::CHUNK_SIZE_IN_BYTE){
                        // std::cout<<"execute 180"<<std::endl;
                        executeWrite(endBufferHost, false);
                        endBufferHost=0;
                    }
                }

            }while(!last);
            std::cout<<"finished write"<<std::endl;
        });

        statusBufferHost[0]=0;
        std::thread chunkReader([&chunkReaderQueue, &chunkReaderKernel, &outputSizeBufferHost, &statusBufferHost, &outputBufferHost, &outputBuffer, &outputSizeBuffer, &statusBuffer, &last]{
            do{
                std::cout<<"start to read a chunk["<<outputBufferHost.size()<<" Bytes]"<<std::endl;
                chunkReaderQueue->enqueueMigrateMemObjects({*statusBuffer}, 0);

                chunkReaderQueue->enqueueTask(*chunkReaderKernel);

                chunkReaderQueue->enqueueMigrateMemObjects({*outputSizeBuffer, *statusBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
                chunkReaderQueue->finish();

                chunkReaderQueue->enqueueMigrateMemObjects({*outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
                chunkReaderQueue->finish();
                internalZstd::decompressQueueOutput.push(outputBufferHost.data(), outputSizeBufferHost[0], last);

                std::cout<<"read a chunk["<<outputSizeBufferHost[0]<<" Bytes]"<<std::endl;
            }while(!statusBufferHost[0]);
            std::cout<<"finished read"<<std::endl;
        });

        chunkWriter.join();
        chunkReader.join();
    }while(!last);
}

// uint64_t zstdCompress(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     uint64_t enbytes = internalZstd::zstdCompressEngineStream(in, out, inputSize);
//     return enbytes;
// }

// uint64_t zstdDecompress(uint8_t* in, uint8_t* out, uint64_t inputSize){
//     uint64_t debytes = internalZstd::zstdDecompressEngineStream(in, out, inputSize);
//     return debytes;
// }

void zstdCompressionInput(uint8_t *in, uint32_t inputSize, bool last){
    if(internalZstd::compressFunc==nullptr){
        internalZstd::compressFunc.reset(nullptr);
        internalZstd::compressFunc=std::make_unique<std::thread>(&zstdCompressEngineStream);

        internalZstd::compressInputs.resize(0);
        internalZstd::compressCur=internalZstd::compressIdx=0;
    }

    uint32_t compressIdx=internalZstd::compressIdx++;
    internalZstd::compressInputs.push_back(std::make_unique<std::thread>([in, inputSize, last, compressIdx]{
        std::cout<<"write thread "<<compressIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalZstd::compressMtx);
        internalZstd::compressCV.wait(lck, [compressIdx] {return internalZstd::compressCur==compressIdx;});

        std::cout<<"write thread "<<compressIdx<<" start to write"<<std::endl;
        internalZstd::compressQueueInput.push(in, inputSize, last);

        internalZstd::compressCur++;
        internalZstd::compressCV.notify_all();
        std::cout<<"write thread "<<compressIdx<<" finished"<<std::endl;
    }));
}

uint32_t zstdCompressionOutput(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size=internalZstd::compressQueueOutput.pop(out, outputSize, last);

    if(last){
        for(auto &t : internalZstd::compressInputs) t->join();
        internalZstd::compressFunc->join();

        internalZstd::compressFunc.reset(nullptr);
        internalZstd::compressInputs.resize(0);
        internalZstd::compressCur=internalZstd::compressIdx=0;
    }

    return size;
}

void zstdDecompressionInput(uint8_t *in, uint32_t inputSize, bool last){
    if(internalZstd::decompressFunc==nullptr){
        internalZstd::decompressFunc.reset(nullptr);
        internalZstd::decompressFunc=std::make_unique<std::thread>(&zstdDecompressEngineStream);

        internalZstd::decompressInputs.resize(0);
        internalZstd::decompressCur=internalZstd::decompressIdx=0;
    }

    uint32_t decompressIdx=internalZstd::decompressIdx++;
    internalZstd::decompressInputs.push_back(std::make_unique<std::thread>([in, inputSize, last, decompressIdx]{
        std::cout<<"write thread "<<decompressIdx<<" get in the function"<<std::endl;
        std::unique_lock<std::mutex> lck(internalZstd::decompressMtx);
        internalZstd::decompressCV.wait(lck, [decompressIdx] {return internalZstd::decompressCur==decompressIdx;});

        std::cout<<"write thread "<<decompressIdx<<" start to write"<<std::endl;
        internalZstd::decompressQueueInput.push(in, inputSize, last);

        internalZstd::decompressCur++;
        internalZstd::decompressCV.notify_all();
        std::cout<<"write thread "<<decompressIdx<<" finished"<<std::endl;
    }));
}

uint32_t zstdDecompressionOutput(uint8_t *out, uint32_t outputSize, bool &last){
    uint32_t size=internalZstd::decompressQueueOutput.pop(out, outputSize, last);

    if(last){
        for(auto &t : internalZstd::decompressInputs) t->join();
        internalZstd::decompressFunc->join();

        internalZstd::decompressFunc.reset(nullptr);
        internalZstd::decompressInputs.resize(0);
        internalZstd::decompressCur=internalZstd::decompressIdx=0;
    }

    return size;
}
}