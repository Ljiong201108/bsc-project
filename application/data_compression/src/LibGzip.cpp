#include "LibGzip.hpp"

namespace data_compression{
namespace gzip{
GzipZlibCompressWorkshop *compressionWorkShop=nullptr;
GzipZlibDecompressWorkshop *decompressionWorkshop=nullptr;
std::thread *processingThread=nullptr;

uint32_t writeGzipHeader(uint8_t *out){
	uint32_t outIdx = 0;
	long int magic_headers = 0x0000000008088B1F;
	std::memcpy(out + outIdx, &magic_headers, 8);
	outIdx += 8;

	long int osheader = 0x00780300;
	std::memcpy(out + outIdx, &osheader, 4);
	outIdx += 4;

	return outIdx;
}

uint32_t writeGzipFooter(uint8_t *out, uint32_t fileSize) {
	uint32_t outIdx = 0;

	// assert(compressionWorkShop!=nullptr && "compression should be executed!");

	// out[outIdx++] = compressionWorkShop->getChecksum();
	// out[outIdx++] = compressionWorkShop->getChecksum() >> 8;
	// out[outIdx++] = compressionWorkShop->getChecksum() >> 16;
	// out[outIdx++] = compressionWorkShop->getChecksum() >> 24;

	out[outIdx++] = 0;
	out[outIdx++] = 0;
	out[outIdx++] = 0;
	out[outIdx++] = 0;

	out[outIdx++] = fileSize;
	out[outIdx++] = fileSize >> 8;
	out[outIdx++] = fileSize >> 16;
	out[outIdx++] = fileSize >> 24;

	// delete compressionWorkShop;
	// compressionWorkShop=nullptr;
	// delete processingThread;
	// processingThread=nullptr;

	return outIdx;
}

bool checkGzipHeader(uint8_t* in){
	uint8_t hidx = 0;
	if (in[hidx++] == 0x1F && in[hidx++] == 0x8B) {
		// Check for magic header
		// Check if method is deflate or not
		if (in[hidx++] != 0x08) {
			std::cerr << "\n";
			std::cerr << "Deflate Header Check Fails" << std::endl;
			return 0;
		}

		// Check if the FLAG has correct value
		// Supported file name or no file name
		// 0x00: No File Name
		// 0x08: File Name
		if (in[hidx] != 0 && in[hidx] != 0x08) {
			std::cerr << "\n";
			std::cerr << "Deflate -n option check failed" << std::endl;
			return 0;
		}
		hidx++;

		// Skip time stamp bytes
		// time stamp contains 4 bytes
		hidx += 4;

		// One extra 0  ending byte
		hidx += 1;

		// Check the operating system code
		// for Unix its 3
		uint8_t oscode_in = in[hidx];
		std::vector<uint8_t> oscodes{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
		bool ochck = std::find(oscodes.cbegin(), oscodes.cend(), oscode_in) == oscodes.cend();
		if (ochck) {
			std::cerr << "\n";
			std::cerr << "GZip header mismatch: OS code is unknown" << std::endl;
			return 0;
		}
	} else {
		hidx = 0;
		// ZLIB Header Checks
		// CMF
		// FLG
		uint8_t cmf = 0x78;
		// 0x01: Fast Mode
		// 0x5E: 1 to 5 levels
		// 0x9C: Default compression: level 6
		// 0xDA: High compression
		std::vector<uint8_t> zlib_flags{0x01, 0x5E, 0x9C, 0xDA};
		if (in[hidx++] == cmf) {
			uint8_t flg = in[hidx];
			bool hchck = std::find(zlib_flags.cbegin(), zlib_flags.cend(), flg) == zlib_flags.cend();
			if (hchck) {
				std::cerr << "\n";
				std::cerr << "Header check fails" << std::endl;
				return 0;
			}
		} else {
			std::cerr << "\n";
			std::cerr << "Zlib Header mismatch" << std::endl;
			return 0;
		}
	}
	return true;
}

void pushGzipCompression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			compressionWorkShop=new GzipZlibCompressWorkshop(false);
			compressionWorkShop->wait();
		});
	}
	while(compressionWorkShop==nullptr);
	compressionWorkShop->getInputStream().push(src, size, last);
}

uint32_t popGzipCompression(void *dest, uint32_t size, bool &last){
	while(compressionWorkShop==nullptr);
	uint32_t ret=compressionWorkShop->getOutputStream().pop(dest, size, last);
	if(last) processingThread->join();
	return ret;
}

void pushGzipDecompression(void* src, uint32_t size, bool first, bool last){
	if(first){
		processingThread=new std::thread([]{
			decompressionWorkshop=new GzipZlibDecompressWorkshop;
			decompressionWorkshop->wait();
		});
	}
	while(decompressionWorkshop==nullptr);
	decompressionWorkshop->getInputStream().push(src, size, last);
}

uint32_t popGzipDecompression(void *dest, uint32_t size, bool &last){
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

} //end gzip
} //end data_compression