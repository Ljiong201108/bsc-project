#pragma once

#include "DataCompression.hpp"
#include <boost/assert.hpp>

namespace dataCompression{
uint32_t writeGzipHeader(uint8_t* out);
uint32_t writeGzipFooter(uint8_t *out, uint32_t fileSize);
uint32_t writeZlibHeader(uint8_t *out);
uint32_t writeZlibFooter(uint8_t *out, uint32_t idxAfterCompress);
bool checkGzipZlibHeader(uint8_t* in);
void gzipZlibCompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last, bool isZlib=false);
void gzipZlibCompressionInput(uint8_t *in, uint32_t inputSize, bool last, bool isZlib=false);
uint32_t gzipZlibCompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last, bool isZlib=false);
uint32_t gzipZlibCompressionOutput(uint8_t *out, uint32_t outputSize, bool &last, bool isZlib=false);
void gzipZlibDecompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last, bool isZlib=false);
void gzipZlibDecompressionInput(uint8_t *in, uint32_t inputSize, bool last, bool isZlib=false);
uint32_t gzipZlibDecompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last, bool isZlib=false);
uint32_t gzipZlibDecompressionOutput(uint8_t *out, uint32_t outputSize, bool &last, bool isZlib=false);
} //dataCompression