#pragma once

#include "DataCompression.hpp"

// Maximum host buffer used to operate per kernel invocation
const auto HOST_BUFFER_SIZE = (32 * 1024 * 1024);

// Default block size
const auto BLOCK_SIZE_IN_KB = 64;

// Maximum number of blocks based on host buffer size
const auto MAX_NUMBER_BLOCKS = (HOST_BUFFER_SIZE / (BLOCK_SIZE_IN_KB * 1024));

namespace dataCompression{
namespace internal{
uint8_t writeHeader(uint8_t* out);
uint8_t readHeader(uint8_t* in);
uint64_t snappyCompressEngineMM(uint8_t* in, uint8_t* out, uint64_t input_size);
uint64_t snappyCompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size);
uint64_t snappyDecompressEngineMM(uint8_t* in, uint8_t* out, size_t input_size);
uint64_t snappyDecompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size);
uint64_t snappyCompressMM(uint8_t* in, uint8_t* out, uint64_t input_size);
uint64_t snappyCompressStream(uint8_t* in, uint8_t* out, uint64_t input_size);
uint64_t snappyDecompressMM(uint8_t* in, uint8_t* out, uint64_t input_size);
uint64_t snappyDecompressStream(uint8_t* in, uint8_t* out, uint64_t input_size);
} //internal
} //dataCompression