#include "SnappyDecompress.hpp"
#include <bitset>

void SnappyDecompressWorkshop::process(){
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

    inputStream.pop(inputBufferHost.data(), 10, last);
    uint8_t streamIdentifier[]={0xff, 0x06, 0x00, 0x00, 0x73, 0x4e, 0x61, 0x50, 0x70, 0x59};
    if(std::memcmp(streamIdentifier, inputBufferHost.data(), 10)!=0){
        std::cerr<<"stream identifier invalid!"<<std::endl;
        exit(EXIT_FAILURE);
    }

    while(!last){
        uint8_t chunkIdentifier;
		inputStream.pop(&chunkIdentifier, 1, last);

        if(chunkIdentifier==0x00){
            uint32_t b1=0, b2=0, b3=0;
			inputStream.pop(&b1, 1, last);
            inputStream.pop(&b2, 1, last);
            inputStream.pop(&b3, 1, last);

            uint32_t blockSize=b1+(b2<<8)+(b3<<16);
            std::cout<<"0x00 block with size: "<<blockSize<<std::endl;

            uint32_t checksum;
            inputStream.pop(&checksum, 4, last);
            blockSize-=4;

            inputStream.pop(inputBufferHost.data()+compressedBlockNum*block_size_in_byte, blockSize, last);
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
            uint32_t b1=0, b2=0, b3=0;
			inputStream.pop(&b1, 1, last);
            inputStream.pop(&b2, 1, last);
            inputStream.pop(&b3, 1, last);

            uint32_t blockSize=b1+(b2<<8)+(b3<<16);
            std::cout<<"0x01 block with size: "<<blockSize<<std::endl;

            uint32_t checksum;
            inputStream.pop(&checksum, 4, last);
            blockSize-=4;

            inputStream.pop(notCompressedBufferHost.data()+notCompressedBlockNum*block_size_in_byte, blockSize, last);
            notCompressedSizeBufferHost[notCompressedBlockNum]=blockSize;
            isCompressed[blockNum]=0;
            notCompressedBlockNum++;
            blockNum++;
        }else if(chunkIdentifier==0xff){
            inputStream.pop(inputBufferHost.data(), 9, last);
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

            for(uint32_t i=0;i<compressedBlockNum;i++){
                std::cout<<i<<" "<<decompressedSizeBufferHost[i]<<std::endl;
            }
            std::cout<<std::endl;

            uint32_t compressedIdx=0, notCompressedIdx=0;
            for(uint32_t blockIdx=0;blockIdx<blockNum;blockIdx++){
                if(isCompressed[blockIdx]){
                    uint32_t blockSize=decompressedSizeBufferHost[compressedIdx];
                    std::cout<<"compressed block: "<<compressedIdx<<", block size is "<<blockSize<<std::endl;
                    outputStream.push(outputBufferHost.data()+compressedIdx*block_size_in_byte, blockSize, blockIdx==blockNum-1 ? last : false);
                    compressedIdx++;
                }else{
                    uint32_t blockSize=notCompressedSizeBufferHost[notCompressedIdx];
                    std::cout<<"not compressed block: "<<compressedIdx<<", block size is "<<blockSize<<std::endl;
                    outputStream.push(notCompressedBufferHost.data()+notCompressedIdx*block_size_in_byte, blockSize, blockIdx==blockNum-1 ? last : false);
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

SnappyDecompressWorkshop::SnappyDecompressWorkshop() : 
	Workshop("SnappyInputStream", 16, "SnappyOutputStream", 16),
	processThread(&SnappyDecompressWorkshop::process, this){}

void SnappyDecompressWorkshop::wait(){
	processThread.join();
}

ByteStream& SnappyDecompressWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& SnappyDecompressWorkshop::getOutputStream(){
	return outputStream;
}