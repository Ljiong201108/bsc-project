#pragma once

#include "Lz4Compress.hpp"
#include "Lz4Decompress.hpp"
#include "lz4_specs.hpp"
#include "xxhash.h"

namespace data_compression{
namespace lz4{

uint8_t writeLz4Header(uint8_t* out, uint64_t inputSize);
uint8_t writeLz4Footer(uint8_t* out);

void pushLz4Compression(void* src, uint32_t size, bool first, bool last);
uint32_t popLz4Compression(void *dest, uint32_t size, bool &last);
void pushLz4Decompression(void* src, uint32_t size, bool first, bool last);
uint32_t popLz4Decompression(void *dest, uint32_t size, bool &last);

} // end lz4
} // end data_compression