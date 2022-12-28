#pragma once

#include "DataCompression.hpp"

namespace dataCompression{
// namespace internalZstd{
// uint64_t zstdCompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize);
// }
// void zstdDecompressEngineStream();
// uint64_t zstdCompress(uint8_t* in, uint8_t* out, uint64_t inputSize);
// uint64_t zstdDecompress(uint8_t* in, uint8_t* out, uint64_t inputSize);
void zstdCompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last);
void zstdCompressionInput(uint8_t *in, uint32_t inputSize, bool last);
uint32_t zstdCompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last);
uint32_t zstdCompressionOutput(uint8_t *out, uint32_t outputSize, bool &last);
void zstdDecompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last);
void zstdDecompressionInput(uint8_t *in, uint32_t inputSize, bool last);
uint32_t zstdDecompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last);
uint32_t zstdDecompressionOutput(uint8_t *out, uint32_t outputSize, bool &last);
}