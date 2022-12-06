#pragma once

#include "DataCompression.hpp"
#include "lz4_specs.hpp"
#include "xxhash.h"

namespace dataCompression{
namespace internal{
// Maximum host buffer used to operate per kernel invocation
const auto HOST_BUFFER_SIZE = (32 * 1024 * 1024);

// Default block size
const auto BLOCK_SIZE_IN_KB = 64;

// Maximum number of blocks based on host buffer size
const auto MAX_NUMBER_BLOCKS = (HOST_BUFFER_SIZE / (BLOCK_SIZE_IN_KB * 1024));

template <typename T>
void writeCompressedBlock(size_t input_size, uint32_t block_length, uint32_t* compressSize, uint8_t* out, T* in, T* buf_out, uint64_t& outIdx, uint64_t& inIdx) {
    uint32_t no_blocks = (input_size - 1) / block_length + 1;
    uint32_t idx = 0;
    for (uint32_t bIdx = 0; bIdx < no_blocks; bIdx++, idx += block_length) {
        // Default block size in bytes i.e., 64 * 1024
        uint32_t block_size = block_length;
        if (idx + block_size > input_size) {
            block_size = input_size - idx;
        }
        uint32_t compressed_size = compressSize[bIdx];
        assert(compressed_size != 0);

        int orig_block_size = input_size;
        int perc_cal = orig_block_size * 10;
        perc_cal = perc_cal / block_size;

        if (compressed_size < block_size && perc_cal >= 10) {
            memcpy(&out[outIdx], &compressed_size, 4);
            outIdx += 4;
            std::memcpy(&out[outIdx], &(buf_out[bIdx * block_length]), compressed_size);
            outIdx += compressed_size;
        } else {
            // No Compression, so copy raw data
            if (block_size == 65536) {
                out[outIdx++] = 0;
                out[outIdx++] = 0;
                out[outIdx++] = 1;
                out[outIdx++] = 128;
            } else {
                uint8_t temp = 0;
                temp = block_size;
                out[outIdx++] = temp;
                temp = block_size >> 8;
                out[outIdx++] = temp;
                out[outIdx++] = 0;
                out[outIdx++] = 128;
            }
            std::memcpy(&out[outIdx], &in[inIdx + idx], block_size);
            outIdx += block_size;
        }
    }
}

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
}// dataCompression