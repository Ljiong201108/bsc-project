#pragma once

#include "DataCompression.hpp"

namespace dataCompression{
namespace internalZstd{
const uint64_t CHUNK_SIZE_IN_KB=16;
const uint64_t CHUNK_SIZE_IN_BYTE=CHUNK_SIZE_IN_KB*1024;

uint64_t zstdCompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize);
uint64_t zstdDecompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize);
}
uint64_t zstdCompress(uint8_t* in, uint8_t* out, uint64_t inputSize);
uint64_t zstdDecompress(uint8_t* in, uint8_t* out, uint64_t inputSize);
}