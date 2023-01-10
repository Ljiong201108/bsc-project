#include "LibZlib.hpp"

namespace data_compression{
namespace zlib{

GzipZlibCompressionWorkshop *compressionWorkShop=nullptr;
GzipZlibDecompressionWorkshop *decompressionWorkshop=nullptr;
std::thread *processingThread=nullptr;

uint32_t writeZlibHeader(uint8_t *out){
	uint32_t outIdx = 0;
	uint32_t m_windowbits=15;
	uint32_t m_level=1;
	uint32_t m_strategy=0;

	// Compression method
	uint8_t CM = DEFLATE_METHOD;

	// Compression Window information
	uint8_t CINFO = m_windowbits - 8;

	// Create CMF header
	uint16_t header = (CINFO << 4);
	header |= CM;
	header <<= 8;

	if (m_level < 2 || m_strategy > 2)
		m_level = 0;
	else if (m_level < 6)
		m_level = 1;
	else if (m_level == 6)
		m_level = 2;
	else
		m_level = 3;

	// CreatE FLG header based on level
	// Strategy information
	header |= (m_level << 6);

	// Ensure that Header (CMF + FLG) is
	// divisible by 31
	header += 31 - (header % 31);

	out[outIdx] = (uint8_t)(header >> 8);
	out[outIdx + 1] = (uint8_t)(header);
	outIdx += 2;

	return outIdx;
}

uint32_t writeZlibFooter(uint8_t *out){
	uint32_t outIdx = 0;

	out[outIdx++] = compressionWorkShop->checksum() >> 24;
	out[outIdx++] = compressionWorkShop->checksum() >> 16;
	out[outIdx++] = compressionWorkShop->checksum() >> 8;
	out[outIdx++] = compressionWorkShop->checksum();

	return outIdx;
}

void pushZlibCompression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			compressionWorkShop=new GzipZlibCompressionWorkshop(true);
			compressionWorkShop->wait();
		});
	}
	while(compressionWorkShop==nullptr);
	compressionWorkShop->getInputStream().push(src, size, last);
}

uint32_t popZlibCompression(void *dest, uint32_t size, bool &last){
	while(compressionWorkShop==nullptr);
	uint32_t ret=compressionWorkShop->getOutputStream().pop(dest, size, last);
	if(last) processingThread->join();
	return ret;
}

void pushZlibDecompression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			decompressionWorkshop=new GzipZlibDecompressionWorkshop;
			decompressionWorkshop->wait();
		});
	}
	while(decompressionWorkshop==nullptr);
	decompressionWorkshop->getInputStream().push(src, size, last);
}

uint32_t popZlibDecompression(void *dest, uint32_t size, bool &last){
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

} //end namespace zlib
} //end namespace data_compression