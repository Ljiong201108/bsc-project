#pragma once

#include "GzipZlibCompress.hpp"
#include "GzipZlibDecompress.hpp"

namespace data_compression{
namespace gzip{
uint32_t writeGzipHeader(uint8_t* out);
uint32_t writeGzipFooter(uint8_t *out, uint32_t fileSize);
bool checkGzipHeader(uint8_t* in);

void pushGzipCompression(void* src, uint32_t size, bool first, bool last);
uint32_t popGzipCompression(void *dest, uint32_t size, bool &last);
void pushGzipDecompression(void* src, uint32_t size, bool first, bool last);
uint32_t popGzipDecompression(void *dest, uint32_t size, bool &last);

} //end gzip
} //end data_compression