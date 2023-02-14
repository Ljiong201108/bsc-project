#include "Lz4Decompress.hpp"
#include <bitset>

void Lz4DecompressWorkshop::process(){
	Timer::startAnaTimer();
	uint64_t hostBufferSize = HOST_BUFFER_SIZE;
    std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(hostBufferSize);
    std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(hostBufferSize);

	Timer::startFPGAInitTimer();
    CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
    KernelPointer decompress_kernel_lz4(Application::getProgram<Lib::LZ4>(), "xilLz4DecompressMM:{xilLz4DecompressMM_1}");
	Timer::endFPGAInitTimer();

    bool last;
    
    do{ // per frames
        inputStream.pop(inBufferHost.data(), 4, last);
        std::cout<<std::hex<<"magic number: "<<*(uint32_t*)inBufferHost.data()<<std::endl;
        if(*(uint32_t*)inBufferHost.data()==0x184D2204){
            //General Structure of LZ4 Frame format
            std::cout<<"General Structure of LZ4 Frame format"<<std::endl;

            //FLG
            inputStream.pop(inBufferHost.data(), 1, last);
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
            inputStream.pop(inBufferHost.data(), 1, last);
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
            // std::cout<<"block_size_in_bytes: "<<block_size_in_bytes<<std::endl;

            uint32_t maxNumBlocks=hostBufferSize/block_size_in_bytes;
            std::vector<uint32_t, aligned_allocator<uint32_t>> decompressSizeBufferHost(maxNumBlocks);
            std::vector<uint32_t, aligned_allocator<uint32_t>> compressSizeBufferHost(maxNumBlocks);

			Timer::startFPGAInitTimer();
            // Device buffer allocation
            BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, hostBufferSize, inBufferHost.data());
            BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, hostBufferSize, outBufferHost.data());
            BufferPointer decompressdSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t)*maxNumBlocks, decompressSizeBufferHost.data());
            BufferPointer compressdSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t)*maxNumBlocks, compressSizeBufferHost.data());
			Timer::endFPGAInitTimer();

            //content size
            if(contentSizeFlg) inputStream.pop(inBufferHost.data(), 4, last);

            //HC
            inputStream.pop(inBufferHost.data(), 1, last);

            //data blocks
            inputStream.pop(inBufferHost.data(), 4, last);

            uint32_t bufferTotalBlock=0;

            auto executeWrite=[&]{
                // std::cout<<"start for decompressing blocks"<<std::endl;
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

                // for(uint32_t i=0;i<bufferTotalBlock;i++){
                //     std::cout<<i<<" "<<decompressSizeBufferHost[i]<<std::endl;
                // }

                for(uint32_t blockIdx=0, decompressedSize=0;blockIdx<bufferTotalBlock;blockIdx++){
                    uint32_t blockSize=decompressSizeBufferHost[blockIdx];
                    outputStream.push(outBufferHost.data()+decompressedSize, blockSize, false);
                    // std::cout<<"decompressed a block"<<std::endl;
                    // hexdump(outBufferHost.data()+decompressedSize, blockSize);
                    decompressedSize+=blockSize;
                }

                bufferTotalBlock=0;
            };

            while(*(uint32_t*)(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes)!=0x0){
                uint32_t compressedSize=*(uint32_t*)(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes);
                if(compressedSize>>31) compressedSize=compressedSize&0x7FFFFFFF;
                // std::cout<<"trying to read a block, size: "<<std::dec<<compressedSize<<std::endl;

                compressSizeBufferHost[bufferTotalBlock]=compressedSize;
                inputStream.pop(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes, compressedSize, last);
                // hexdump(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes, compressedSize);
                bufferTotalBlock++;

                //这里必须要减一，不然decompressedSize数据前面全是0，就最后一个是65536
				Timer::startFPGAInitTimer();
                if(bufferTotalBlock==maxNumBlocks-1) executeWrite();
				Timer::endFPGAInitTimer();
                
                uint32_t _;
                if(blockChecksumFlg) inputStream.pop(&_, 4, last);
                
                // std::cout<<"finished reading a block"<<std::endl;
                //next data block size
                inputStream.pop(inBufferHost.data()+bufferTotalBlock*block_size_in_bytes, 4, last);
            }

			Timer::startFPGAInitTimer();
            if(bufferTotalBlock) executeWrite();
			Timer::endFPGAInitTimer();

            if(contentChecksumFlg) inputStream.pop(inBufferHost.data(), 4, last);
        }else if(0x184D2A50<=*(uint32_t*)inBufferHost.data() && *(uint32_t*)inBufferHost.data()<=0x184D2A5F){
            uint32_t size;
            inputStream.pop(&size, 4, last);

			uint8_t temp;
            while(size--) inputStream.pop(&temp, 1, last);
        }

        //结束标识出现在输出之后，无法提前得知，所以需要多写一个，会在读出的时候被去掉
        if(last){
			uint8_t temp=0;
            outputStream.push(&temp, 1, true);
            // std::cout<<"finish decompressing"<<std::endl;
        }   

        // std::cout<<"frame end"<<std::endl;
    }while(!last);
	Timer::endAnaTimer();
}

Lz4DecompressWorkshop::Lz4DecompressWorkshop() : 
	Workshop("Lz4InputStream", 1<<30, "Lz4OutputStream", 1<<30),
	processThread(&Lz4DecompressWorkshop::process, this){}

void Lz4DecompressWorkshop::wait(){
	processThread.join();
}

ByteStream& Lz4DecompressWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& Lz4DecompressWorkshop::getOutputStream(){
	return outputStream;
}