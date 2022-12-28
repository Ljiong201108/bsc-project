#pragma once

#include "DataCompression.hpp"

namespace dataCompression{
namespace internalSnappy{
    
// Maximum host buffer used to operate per kernel invocation
const auto HOST_BUFFER_SIZE = (64 * 1024 * 1024);

// Default block size
const auto BLOCK_SIZE_IN_KB = 64;

// Maximum number of blocks based on host buffer size
const auto MAX_NUMBER_BLOCKS = (HOST_BUFFER_SIZE / (BLOCK_SIZE_IN_KB * 1024));

} //internal
uint8_t writeSnappyHeader(uint8_t* out);
uint64_t snappyCompressEngineMM(uint8_t* in, uint8_t* out, uint64_t inputSize);
uint64_t snappyCompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize);
uint64_t snappyDecompressEngineMM(uint8_t* in, uint8_t* out, uint64_t inputSize);
uint64_t snappyDecompressEngineStream(uint8_t* in, uint8_t* out, uint64_t inputSize);
// uint64_t snappyCompressMM(uint8_t* in, uint8_t* out, uint64_t inputSize);
// uint64_t snappyCompressStream(uint8_t* in, uint8_t* out, uint32_t inputSize);
// uint64_t snappyDecompressMM(uint8_t* in, uint8_t* out, uint64_t inputSize);
// uint64_t snappyDecompressStream(uint8_t* in, uint8_t* out, uint32_t inputSize);

// uint64_t snappyCompress(uint8_t* in, uint8_t* out, uint64_t inputSize, bool stream=false);
// uint64_t snappyDecompress(uint8_t* in, uint8_t* out, uint64_t inputSize, bool stream=false);

void snappyCompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last);
void snappyCompressionInput(uint8_t *in, uint32_t inputSize, bool last);
uint32_t snappyCompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last);
uint32_t snappyCompressionOutput(uint8_t *out, uint32_t outputSize, bool &last);
void snappyDecompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last);
void snappyDecompressionInput(uint8_t *in, uint32_t inputSize, bool last);
uint32_t snappyDecompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last);
uint32_t snappyDecompressionOutput(uint8_t *out, uint32_t outputSize, bool &last);
} //dataCompression