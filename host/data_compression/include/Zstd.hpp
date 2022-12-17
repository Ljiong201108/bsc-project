#pragma once

#include "DataCompression.hpp"

namespace dataCompression{
namespace internalZstd{
uint64_t zstdCompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize);
}
void zstdDecompressEngineStream();
uint64_t zstdCompress(uint8_t* in, uint8_t* out, uint64_t inputSize);
// uint64_t zstdDecompress(uint8_t* in, uint8_t* out, uint64_t inputSize);
void zstdDecompressionInput(uint8_t *in, uint32_t inputSize, bool last);
uint32_t zstdDecompressionOutput(uint8_t *out, uint32_t outputSize, bool &last);
}