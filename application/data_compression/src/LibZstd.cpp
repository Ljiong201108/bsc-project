#include "LibZstd.hpp"

namespace data_compression{
namespace zstd{
ZstdCompressWorkshop *compressionWorkShop=nullptr;
ZstdDecompressWorkshop *decompressionWorkshop=nullptr;
std::thread *processingThread=nullptr;

void pushZstdCompression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			compressionWorkShop=new ZstdCompressWorkshop;
			compressionWorkShop->wait();
		});
	}
	while(compressionWorkShop==nullptr);
	compressionWorkShop->getInputStream().push(src, size, last);
}

uint32_t popZstdCompression(void *dest, uint32_t size, bool &last){
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

void pushZstdDecompression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			decompressionWorkshop=new ZstdDecompressWorkshop;
			decompressionWorkshop->wait();
		});
	}
	while(decompressionWorkshop==nullptr);
	decompressionWorkshop->getInputStream().push(src, size, last);
}

uint32_t popZstdDecompression(void *dest, uint32_t size, bool &last){
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

} //end zstd
} //end data_compression