#pragma once

#include "SnappyCompress.hpp"
#include "SnappyDecompress.hpp"
#include "lz4_specs.hpp"
#include "xxhash.h"

namespace data_compression{
namespace snappy{

uint8_t writeSnappyHeader(uint8_t* out);

void pushSnappyCompression(void* src, uint32_t size, bool first, bool last);
uint32_t popSnappyCompression(void *dest, uint32_t size, bool &last);
void pushSnappyDecompression(void* src, uint32_t size, bool first, bool last);
uint32_t popSnappyDecompression(void *dest, uint32_t size, bool &last);

} // end snappy
} // end data_compression