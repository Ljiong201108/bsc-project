#pragma once

#include "GzipZlibCompress.hpp"
#include "GzipZlibDecompress.hpp"

namespace data_compression{
namespace zlib{
constexpr auto DEFLATE_METHOD = 8;

uint32_t writeZlibHeader(uint8_t *out);
uint32_t writeZlibFooter(uint8_t *out, uint32_t idxAfterCompress);

void pushZlibCompression(void* src, uint32_t size, bool first, bool last);
uint32_t popZlibCompression(void *dest, uint32_t size, bool &last);
void pushZlibDecompression(void* src, uint32_t size, bool first, bool last);
uint32_t popZlibDecompression(void *dest, uint32_t size, bool &last);
} //end namespace zlib
} //end namespace data_compression