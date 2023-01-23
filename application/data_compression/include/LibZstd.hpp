#pragma once

#include "ZstdCompress.hpp"
#include "ZstdDecompress.hpp"
#include "lz4_specs.hpp"
#include "xxhash.h"

namespace data_compression{
namespace zstd{
void pushZstdCompression(void* src, uint32_t size, bool first, bool last);
uint32_t popZstdCompression(void *dest, uint32_t size, bool &last);
void pushZstdDecompression(void* src, uint32_t size, bool first, bool last);
uint32_t popZstdDecompression(void *dest, uint32_t size, bool &last);

} // end zstd
} // end data_compression