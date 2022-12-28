#pragma once

#include "DataCompression.hpp"
#include "lz4_specs.hpp"
#include "xxhash.h"

namespace dataCompression{
namespace internalLz4{

//writeCompressedBlock<uint8_t>(hostChunk_cu, block_size_in_bytes, h_compressSize.data(), out, in, h_buf_out.data(), outIdx, inIdx);
// template <typename T>
// void writeCompressedBlock(uint64_t hostChunk_cu, uint32_t block_size_in_bytes, uint32_t* h_compressSize.data(), uint8_t* out, T* in, T* h_buf_out.data(), uint64_t& outIdx, uint64_t& inIdx) {
//     uint32_t no_blocks = (hostChunk_cu - 1) / block_size_in_bytes + 1;
//     uint32_t idx = 0;
//     for (uint32_t bIdx = 0; bIdx < no_blocks; bIdx++, idx += block_size_in_bytes) {
//         // Default block size in bytes i.e., 64 * 1024
//         uint32_t block_size = block_size_in_bytes;
//         if (idx + block_size > hostChunk_cu) {
//             block_size = hostChunk_cu - idx;
//         }
//         uint32_t compressed_size = h_compressSize.data()[bIdx];
//         assert(compressed_size != 0);

//         int orig_block_size = hostChunk_cu;
//         int perc_cal = orig_block_size * 10;
//         perc_cal = perc_cal / block_size;

//         if (compressed_size < block_size && perc_cal >= 10) {
//             memcpy(&out[outIdx], &compressed_size, 4);
//             outIdx += 4;
//             std::memcpy(&out[outIdx], &(h_buf_out.data()[bIdx * block_size_in_bytes]), compressed_size);
//             outIdx += compressed_size;
//         } else {
//             // No Compression, so copy raw data
//             if (block_size == 65536) {
//                 out[outIdx++] = 0;
//                 out[outIdx++] = 0;
//                 out[outIdx++] = 1;
//                 out[outIdx++] = 128;
//             } else {
//                 uint8_t temp = 0;
//                 temp = block_size;
//                 out[outIdx++] = temp;
//                 temp = block_size >> 8;
//                 out[outIdx++] = temp;
//                 out[outIdx++] = 0;
//                 out[outIdx++] = 128;
//             }
//             std::memcpy(&out[outIdx], &in[inIdx + idx], block_size);
//             outIdx += block_size;
//         }
//     }
// }

inline uint8_t get_bsize(uint32_t c_input_size) {
    switch (c_input_size) {
        case xf::compression::MAX_BSIZE_64KB:
            return xf::compression::BSIZE_NCOMP_64;
            break;
        case xf::compression::MAX_BSIZE_256KB:
            return xf::compression::BSIZE_NCOMP_256;
            break;
        case xf::compression::MAX_BSIZE_1024KB:
            return xf::compression::BSIZE_NCOMP_1024;
            break;
        case xf::compression::MAX_BSIZE_4096KB:
            return xf::compression::BSIZE_NCOMP_4096;
            break;
        default:
            return xf::compression::BSIZE_NCOMP_64;
            break;
    }
}
}// internal

uint8_t writeLz4Header(uint8_t* out, uint64_t inputSize);
uint8_t writeLz4Footer(uint8_t* out);
uint8_t readLz4Header(uint8_t* in);
// uint64_t lz4Compress(uint8_t* in, uint8_t* out, uint64_t inputSize, bool stream=false);
// uint64_t lz4Decompress(uint8_t* in, uint8_t* out, uint64_t inputSize, bool stream=false);
void lz4CompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last);
void lz4CompressionInput(uint8_t *in, uint32_t inputSize, bool last);
uint32_t lz4CompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last);
uint32_t lz4CompressionOutput(uint8_t *out, uint32_t outputSize, bool &last);
void lz4DecompressionInputAsyc(uint8_t *in, uint32_t inputSize, bool last);
void lz4DecompressionInput(uint8_t *in, uint32_t inputSize, bool last);
uint32_t lz4DecompressionOutputAsyc(uint8_t *out, uint32_t outputSize, bool &last);
uint32_t lz4DecompressionOutput(uint8_t *out, uint32_t outputSize, bool &last);
}// dataCompression