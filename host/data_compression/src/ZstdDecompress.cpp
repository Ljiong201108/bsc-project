#include "ZstdDecompress.hpp"
#include <bitset>

// void ZstdDecompressWorkshop::process(){
// 	CommandQueuePointer chunkWriterQueue(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
//     CommandQueuePointer chunkReaderQueue(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);

//     KernelPointer chunkWriterKernel(Application::getProgram<Lib::ZSTD>(), "xilMM2S:{xilMM2S_1}");
//     KernelPointer chunkReaderKernel(Application::getProgram<Lib::ZSTD>(), "xilS2MM:{xilS2MM_1}");

//     //memory for compression
//     std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(CHUNK_SIZE_IN_BYTE);
//     BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, inputBufferHost.size(), inputBufferHost.data());

//     //memory for decompression
//     std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(CHUNK_SIZE_IN_BYTE);
//     std::vector<uint32_t, aligned_allocator<uint32_t>> outputSizeBufferHost(1);
//     std::vector<uint32_t, aligned_allocator<uint32_t>> statusBufferHost(1);

//     BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint8_t)*outputBufferHost.size(), outputBufferHost.data());
//     BufferPointer outputSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), outputSizeBufferHost.data());
//     BufferPointer statusBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), statusBufferHost.data());

//     //kernel args fixed setting
//     chunkWriterKernel->setArg(0, *inputBuffer);

//     chunkReaderKernel->setArg(0, *outputBuffer);
//     chunkReaderKernel->setArg(1, *outputSizeBuffer);
//     chunkReaderKernel->setArg(2, *statusBuffer);
//     chunkReaderKernel->setArg(3, (uint32_t)outputBufferHost.size());

//     bool last;
//     uint32_t endBufferHost;

//     do{ //per frame
//         endBufferHost=0;
//         std::thread chunkWriter([&]{
//             const uint32_t magicZstandardNumber=0xFD2FB528;
//             const uint32_t magicSkippableNumberMin=0x184D2A50;
//             const uint32_t magicSkippableNumberMax=0x184D2A5F;

//             auto executeWrite=[&](uint32_t size, bool last){
//                 std::cout<<"start writing a block of "<<size<<", last: "<<last<<std::endl;
//                 // hexdump(inputBufferHost.data(), size);

//                 chunkWriterQueue->enqueueMigrateMemObjects({*inputBuffer}, 0);
//                 chunkWriterQueue->finish();

//                 chunkWriterKernel->setArg(1, size);
//                 chunkWriterKernel->setArg(2, (uint32_t)last);

//                 chunkWriterQueue->enqueueTask(*chunkWriterKernel);
//                 chunkWriterQueue->finish();

//                 std::cout<<"write a chunk["<<size<<" Bytes], last = "<<last<<std::endl;
//             };

//             auto safePushBack=[&](uint8_t value, bool last){
//                 *(inputBufferHost.data()+endBufferHost)=value;
//                 endBufferHost++;

//                 if(endBufferHost==CHUNK_SIZE_IN_BYTE || last){
//                     executeWrite(endBufferHost, last);
//                     endBufferHost=0;
//                 }
//             };

// 			auto safePushBackSeg=[&](uint8_t *src, uint32_t size, bool last){
// 				while(size){
// 					uint32_t canPush=std::min(size, (uint32_t)(CHUNK_SIZE_IN_BYTE-endBufferHost));
// 					memcpy(inputBufferHost.data()+endBufferHost, src, canPush);
// 					src+=canPush;
// 					endBufferHost+=canPush;
// 					size-=canPush;

// 					if(size==0 && endBufferHost<CHUNK_SIZE_IN_BYTE)
// 						if(last) executeWrite(endBufferHost, false);
// 					else if(size!=0 && endBufferHost==CHUNK_SIZE_IN_BYTE)
// 						executeWrite(endBufferHost, false);
// 					if(size==0 && endBufferHost==CHUNK_SIZE_IN_BYTE)
// 						executeWrite(endBufferHost, last);
// 				}
// 			};

//             inputStream.pop(inputBufferHost.data(), 4, last);
//             std::cout<<"magic number: "<<std::hex<<*(uint32_t*)inputBufferHost.data()<<std::dec<<std::endl;
//             endBufferHost+=4;
//             if(*(uint32_t*)inputBufferHost.data()==magicZstandardNumber){
//                 // Frame Header Descriptor 
//                 inputStream.pop(inputBufferHost.data()+endBufferHost, 1, last);
//                 uint8_t FHD=*(inputBufferHost.data()+endBufferHost);
//                 endBufferHost++;

//                 uint8_t frameContentSizeFlag=FHD>>6;
//                 uint8_t singleSegmentFlag=(FHD>>5)&0b1;
//                 uint8_t contentChecksumFlag=(FHD>>2)&0b1;
//                 uint8_t dictionaryIDFlag=FHD&0b1;

//                 uint32_t FCSFieldSize;
//                 switch(frameContentSizeFlag){
//                     case 0: FCSFieldSize=singleSegmentFlag ? 1 : 0; break;
//                     case 1: FCSFieldSize=2; break;
//                     case 2: FCSFieldSize=4; break;
//                     case 3: FCSFieldSize=8; break;
//                 }

//                 // Window Descriptor
//                 if(!singleSegmentFlag){
//                     inputStream.pop(inputBufferHost.data()+endBufferHost, 1, last);
//                     endBufferHost++;
//                 }

//                 // std::cout<<"frameContentSizeFlag: "<<(uint32_t)frameContentSizeFlag<<std::endl;
//                 // std::cout<<"singleSegmentFlag: "<<(uint32_t)singleSegmentFlag<<std::endl;
//                 // std::cout<<"contentChecksumFlag: "<<(uint32_t)contentChecksumFlag<<std::endl;
//                 // std::cout<<"dictionaryIDFlag: "<<(uint32_t)dictionaryIDFlag<<std::endl;
//                 // std::cout<<"FCSFieldSize: "<<FCSFieldSize<<std::endl;

//                 // Dictionary ID
//                 if(dictionaryIDFlag){
//                     inputStream.pop(inputBufferHost.data()+endBufferHost, dictionaryIDFlag, last);
//                     endBufferHost+=dictionaryIDFlag;
//                 }

//                 // Frame Content Size
//                 if(FCSFieldSize){
//                     inputStream.pop(inputBufferHost.data()+endBufferHost, FCSFieldSize, last);
//                     endBufferHost+=FCSFieldSize;
//                 }

//                 bool lastBlock;
//                 do{
//                     uint32_t b1=inputStream.pop(last);
//                     uint32_t b2=inputStream.pop(last);
//                     uint32_t b3=inputStream.pop(last);

//                     // std::cout<<std::dec<<b1<<" "<<b2<<" "<<b3<<std::endl;

//                     lastBlock=b1&0b1;
//                     uint32_t blockSize=(b1>>3)+(b2<<5)+((b3)<<13);
//                     // std::cout<<"blockSize: "<<std::dec<<blockSize<<std::endl;

//                     safePushBack(b1, false);
//                     safePushBack(b2, false);
//                     safePushBack(b3, false);

//                     for(uint32_t i=0;i<blockSize-1;i++) safePushBack(inputStream.pop(last), false);
//                     // for the last byte of the block
//                     if(lastBlock && !contentChecksumFlag) safePushBack(inputStream.pop(last), true);
//                     else if(lastBlock && contentChecksumFlag){
//                         safePushBack(inputStream.pop(last), false);
//                         uint32_t checksum;
//                         inputStream.pop(&checksum, 4, last);
//                         safePushBack(*((uint8_t*)(&checksum)+0), false);
//                         safePushBack(*((uint8_t*)(&checksum)+1), false);
//                         safePushBack(*((uint8_t*)(&checksum)+2), false);
//                         safePushBack(*((uint8_t*)(&checksum)+3), true);
//                     }
//                 }while(!lastBlock);

//             }else if(magicSkippableNumberMin<=*(uint32_t*)inputBufferHost.data() && *(uint32_t*)inputBufferHost.data()<=magicSkippableNumberMax){
//                 inputStream.pop(inputBufferHost.data()+endBufferHost, 4, last);
//                 uint32_t frameSize=*(uint32_t*)(inputBufferHost.data()+endBufferHost);
//                 endBufferHost+=4;

//                 for(uint32_t i=0;i<frameSize-1;i++) safePushBack(inputStream.pop(last), false);
//                 safePushBack(inputStream.pop(last), true);
//             }else{
//                 std::cerr<<"magic number invalid!"<<std::endl;
//                 exit(EXIT_FAILURE);
//             }

//             std::cout<<"finished write"<<std::endl;
//         });

//         statusBufferHost[0]=0;
//         std::thread chunkReader([this, &chunkReaderQueue, &chunkReaderKernel, &outputSizeBufferHost, &statusBufferHost, &outputBufferHost, &outputBuffer, &outputSizeBuffer, &statusBuffer, &last]{
//             do{
//                 std::cout<<"start to read a chunk["<<std::dec<<outputBufferHost.size()<<" Bytes]"<<std::endl;
//                 chunkReaderQueue->enqueueMigrateMemObjects({*statusBuffer}, 0);

//                 chunkReaderQueue->enqueueTask(*chunkReaderKernel);

//                 chunkReaderQueue->enqueueMigrateMemObjects({*outputSizeBuffer, *statusBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
//                 chunkReaderQueue->finish();

//                 chunkReaderQueue->enqueueMigrateMemObjects({*outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
//                 chunkReaderQueue->finish();
//                 outputStream.push(outputBufferHost.data(), outputSizeBufferHost[0], last);

//                 std::cout<<"read a chunk["<<outputSizeBufferHost[0]<<" Bytes]"<<std::endl;
//             }while(!statusBufferHost[0]);
//             std::cout<<"finished read"<<std::endl;
//         });

//         chunkWriter.join();
//         chunkReader.join();
//     }while(!last);
// }

void ZstdDecompressWorkshop::process(){
	CommandQueuePointer chunkWriterQueue(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
    CommandQueuePointer chunkReaderQueue(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);

    KernelPointer chunkWriterKernel(Application::getProgram<Lib::ZSTD>(), "xilMM2S:{xilMM2S_1}");
    KernelPointer chunkReaderKernel(Application::getProgram<Lib::ZSTD>(), "xilS2MM:{xilS2MM_1}");

    //memory for compression
    std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(CHUNK_SIZE_IN_BYTE);
    BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, inputBufferHost.size(), inputBufferHost.data());

    //memory for decompression
    std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(CHUNK_SIZE_IN_BYTE);
    std::vector<uint32_t, aligned_allocator<uint32_t>> outputSizeBufferHost(1);
    std::vector<uint32_t, aligned_allocator<uint32_t>> statusBufferHost(1);

    BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint8_t)*outputBufferHost.size(), outputBufferHost.data());
    BufferPointer outputSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), outputSizeBufferHost.data());
    BufferPointer statusBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), statusBufferHost.data());

    //kernel args fixed setting
    chunkWriterKernel->setArg(0, *inputBuffer);

    chunkReaderKernel->setArg(0, *outputBuffer);
    chunkReaderKernel->setArg(1, *outputSizeBuffer);
    chunkReaderKernel->setArg(2, *statusBuffer);
    chunkReaderKernel->setArg(3, (uint32_t)outputBufferHost.size());

    bool last;
    uint32_t endBufferHost;

    do{ //per frame
        endBufferHost=0;
        std::thread chunkWriter([&]{
            const uint32_t magicZstandardNumber=0xFD2FB528;
            const uint32_t magicSkippableNumberMin=0x184D2A50;
            const uint32_t magicSkippableNumberMax=0x184D2A5F;

            auto executeWrite=[&](uint32_t size, bool last){
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

            auto safePushBack=[&](uint8_t value, bool last){
                *(inputBufferHost.data()+endBufferHost)=value;
                endBufferHost++;

                if(endBufferHost==CHUNK_SIZE_IN_BYTE || last){
                    executeWrite(endBufferHost, last);
                    endBufferHost=0;
                }
            };

			auto safePushBackSeg=[&](uint8_t *src, uint32_t size, bool last){
				while(size){
					uint32_t canPush=std::min(size, (uint32_t)(CHUNK_SIZE_IN_BYTE-endBufferHost));
					memcpy(inputBufferHost.data()+endBufferHost, src, canPush);
					src+=canPush;
					endBufferHost+=canPush;
					size-=canPush;

					if(size==0 && endBufferHost<CHUNK_SIZE_IN_BYTE)
						if(last) executeWrite(endBufferHost, false);
					else if(size!=0 && endBufferHost==CHUNK_SIZE_IN_BYTE)
						executeWrite(endBufferHost, false);
					if(size==0 && endBufferHost==CHUNK_SIZE_IN_BYTE)
						executeWrite(endBufferHost, last);
				}
			};

            inputStream.pop(inputBufferHost.data(), 4, last);
            std::cout<<"magic number: "<<std::hex<<*(uint32_t*)inputBufferHost.data()<<std::dec<<std::endl;
            endBufferHost+=4;
            if(*(uint32_t*)inputBufferHost.data()==magicZstandardNumber){
                // Frame Header Descriptor 
                inputStream.pop(inputBufferHost.data()+endBufferHost, 1, last);
                uint8_t FHD=*(inputBufferHost.data()+endBufferHost);
                endBufferHost++;

                uint8_t frameContentSizeFlag=FHD>>6;
                uint8_t singleSegmentFlag=(FHD>>5)&0b1;
                uint8_t contentChecksumFlag=(FHD>>2)&0b1;
                uint8_t dictionaryIDFlag=FHD&0b1;

                uint32_t FCSFieldSize;
                switch(frameContentSizeFlag){
                    case 0: FCSFieldSize=singleSegmentFlag ? 1 : 0; break;
                    case 1: FCSFieldSize=2; break;
                    case 2: FCSFieldSize=4; break;
                    case 3: FCSFieldSize=8; break;
                }

                // Window Descriptor
                if(!singleSegmentFlag){
                    inputStream.pop(inputBufferHost.data()+endBufferHost, 1, last);
                    endBufferHost++;
                }

                // std::cout<<"frameContentSizeFlag: "<<(uint32_t)frameContentSizeFlag<<std::endl;
                // std::cout<<"singleSegmentFlag: "<<(uint32_t)singleSegmentFlag<<std::endl;
                // std::cout<<"contentChecksumFlag: "<<(uint32_t)contentChecksumFlag<<std::endl;
                // std::cout<<"dictionaryIDFlag: "<<(uint32_t)dictionaryIDFlag<<std::endl;
                // std::cout<<"FCSFieldSize: "<<FCSFieldSize<<std::endl;

                // Dictionary ID
                if(dictionaryIDFlag){
                    inputStream.pop(inputBufferHost.data()+endBufferHost, dictionaryIDFlag, last);
                    endBufferHost+=dictionaryIDFlag;
                }

                // Frame Content Size
                if(FCSFieldSize){
                    inputStream.pop(inputBufferHost.data()+endBufferHost, FCSFieldSize, last);
                    endBufferHost+=FCSFieldSize;
                }

                bool lastBlock;
                do{
                    uint32_t b1=inputStream.pop(last);
                    uint32_t b2=inputStream.pop(last);
                    uint32_t b3=inputStream.pop(last);

                    // std::cout<<std::dec<<b1<<" "<<b2<<" "<<b3<<std::endl;

                    lastBlock=b1&0b1;
                    uint32_t blockSize=(b1>>3)+(b2<<5)+((b3)<<13);
                    // std::cout<<"blockSize: "<<std::dec<<blockSize<<std::endl;

                    safePushBack(b1, false);
                    safePushBack(b2, false);
                    safePushBack(b3, false);

                    for(uint32_t i=0;i<blockSize-1;i++) safePushBack(inputStream.pop(last), false);
                    // for the last byte of the block
                    if(lastBlock && !contentChecksumFlag) safePushBack(inputStream.pop(last), true);
                    else if(lastBlock && contentChecksumFlag){
                        safePushBack(inputStream.pop(last), false);
                        uint32_t checksum;
                        inputStream.pop(&checksum, 4, last);
                        safePushBack(*((uint8_t*)(&checksum)+0), false);
                        safePushBack(*((uint8_t*)(&checksum)+1), false);
                        safePushBack(*((uint8_t*)(&checksum)+2), false);
                        safePushBack(*((uint8_t*)(&checksum)+3), true);
                    }
                }while(!lastBlock);

            }else if(magicSkippableNumberMin<=*(uint32_t*)inputBufferHost.data() && *(uint32_t*)inputBufferHost.data()<=magicSkippableNumberMax){
                inputStream.pop(inputBufferHost.data()+endBufferHost, 4, last);
                uint32_t frameSize=*(uint32_t*)(inputBufferHost.data()+endBufferHost);
                endBufferHost+=4;

                for(uint32_t i=0;i<frameSize-1;i++) safePushBack(inputStream.pop(last), false);
                safePushBack(inputStream.pop(last), true);
            }else{
                std::cerr<<"magic number invalid!"<<std::endl;
                exit(EXIT_FAILURE);
            }

            std::cout<<"finished write"<<std::endl;
        });

        statusBufferHost[0]=0;
        std::thread chunkReader([this, &chunkReaderQueue, &chunkReaderKernel, &outputSizeBufferHost, &statusBufferHost, &outputBufferHost, &outputBuffer, &outputSizeBuffer, &statusBuffer, &last]{
            do{
                std::cout<<"start to read a chunk["<<std::dec<<outputBufferHost.size()<<" Bytes]"<<std::endl;
                chunkReaderQueue->enqueueMigrateMemObjects({*statusBuffer}, 0);

                chunkReaderQueue->enqueueTask(*chunkReaderKernel);

                chunkReaderQueue->enqueueMigrateMemObjects({*outputSizeBuffer, *statusBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
                chunkReaderQueue->finish();

                chunkReaderQueue->enqueueMigrateMemObjects({*outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
                chunkReaderQueue->finish();
                outputStream.push(outputBufferHost.data(), outputSizeBufferHost[0], last);

                std::cout<<"read a chunk["<<outputSizeBufferHost[0]<<" Bytes]"<<std::endl;
            }while(!statusBufferHost[0]);
            std::cout<<"finished read"<<std::endl;
        });

        chunkWriter.join();
        chunkReader.join();
    }while(!last);
}

ZstdDecompressWorkshop::ZstdDecompressWorkshop() : 
	Workshop("ZstdInputStream", 16, "ZstdOutputStream", 16),
	processThread(&ZstdDecompressWorkshop::process, this){}

void ZstdDecompressWorkshop::wait(){
	processThread.join();
}

ByteStream& ZstdDecompressWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& ZstdDecompressWorkshop::getOutputStream(){
	return outputStream;
}