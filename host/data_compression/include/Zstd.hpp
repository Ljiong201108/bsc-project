#pragma once

#include "DataCompression.hpp"

namespace dataCompression{
namespace internal{
uint64_t zstdCompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size);
uint64_t zstdDecompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size, uint64_t max_outbuf_size);
uint64_t zstdCompressStream(uint8_t* in, uint8_t* out, size_t input_size);
uint64_t zstdDecompressStream(uint8_t* in, uint8_t* out, size_t input_size, uint64_t maxOutputSize);
}
}