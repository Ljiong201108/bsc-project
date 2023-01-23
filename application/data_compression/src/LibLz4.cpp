#include "LibLz4.hpp"

namespace data_compression{
namespace lz4{
Lz4CompressWorkshop *compressionWorkShop=nullptr;
Lz4DecompressWorkshop *decompressionWorkshop=nullptr;
std::thread *processingThread=nullptr;

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
    switch (Lz4CompressWorkshop::BLOCK_SIZE_IN_KB) {
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

void pushLz4Compression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			compressionWorkShop=new Lz4CompressWorkshop;
			compressionWorkShop->wait();
		});
	}
	while(compressionWorkShop==nullptr);
	compressionWorkShop->getInputStream().push(src, size, last);
}

uint32_t popLz4Compression(void *dest, uint32_t size, bool &last){
	while(compressionWorkShop==nullptr);
	uint32_t ret=compressionWorkShop->getOutputStream().pop(dest, size, last);
	if(last){
		processingThread->join();
		delete decompressionWorkshop;
		decompressionWorkshop=nullptr;
		delete processingThread;
		processingThread=nullptr;
	}
	return ret;
}

void pushLz4Decompression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			decompressionWorkshop=new Lz4DecompressWorkshop;
			decompressionWorkshop->wait();
		});
	}
	while(decompressionWorkshop==nullptr);
	decompressionWorkshop->getInputStream().push(src, size, last);
}

uint32_t popLz4Decompression(void *dest, uint32_t size, bool &last){
	while(decompressionWorkshop==nullptr);
	uint32_t ret=decompressionWorkshop->getOutputStream().pop(dest, size, last);
	if(last){
		processingThread->join();
		delete decompressionWorkshop;
		decompressionWorkshop=nullptr;
		delete processingThread;
		processingThread=nullptr;

		ret--;
	}
	return ret;
}

} //end lz4
} //end data_compression