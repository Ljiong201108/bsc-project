#include "LibSnappy.hpp"

namespace data_compression{
namespace snappy{
SnappyCompressWorkshop *compressionWorkShop=nullptr;
SnappyDecompressWorkshop *decompressionWorkshop=nullptr;
std::thread *processingThread=nullptr;

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

void pushSnappyCompression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			compressionWorkShop=new SnappyCompressWorkshop;
			compressionWorkShop->wait();
		});
	}
	while(compressionWorkShop==nullptr);
	compressionWorkShop->getInputStream().push(src, size, last);
}

uint32_t popSnappyCompression(void *dest, uint32_t size, bool &last){
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

void pushSnappyDecompression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			decompressionWorkshop=new SnappyDecompressWorkshop;
			decompressionWorkshop->wait();
		});
	}
	while(decompressionWorkshop==nullptr);
	decompressionWorkshop->getInputStream().push(src, size, last);
}

uint32_t popSnappyDecompression(void *dest, uint32_t size, bool &last){
	while(decompressionWorkshop==nullptr);
	uint32_t ret=decompressionWorkshop->getOutputStream().pop(dest, size, last);
	if(last){
		processingThread->join();
		delete decompressionWorkshop;
		decompressionWorkshop=nullptr;
		delete processingThread;
		processingThread=nullptr;
	}
	return ret;
}

} //end snappy
} //end data_compression