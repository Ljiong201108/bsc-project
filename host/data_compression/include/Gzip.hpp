#pragma once

#include "DataCompression.hpp"
#include <boost/assert.hpp>

namespace dataCompression{
namespace internal{
uint32_t writeGzipHeader(uint8_t* out);
uint32_t writeGzipFooter(uint8_t* out, uint32_t compressSize, uint32_t checksum, const std::string &fileName, uint32_t fileSize);
bool readGzipHeader(uint8_t* in);

uint32_t gzipCompressionInternal(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t &checksum);
uint32_t gzipDecompressionInternalMM(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t max_output_size);
uint32_t gzipDecompressionInternalStream(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t max_output_size);
}
uint32_t gzipCompression(uint8_t *in, uint8_t *out, uint32_t inputSize, const std::string &fileName);
uint32_t gzipDecompression(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t max_output_size, bool stream=true);
}