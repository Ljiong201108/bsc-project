#pragma once

#include "DataCompression.hpp"
#include <boost/assert.hpp>

constexpr auto page_aligned_mem = (1 << 21);

// Aligned allocator for ZLIB
template <typename T>
struct zlib_aligned_allocator {
    using value_type = T;
    T* allocate(std::size_t num) {
        void* ptr = nullptr;
        if (posix_memalign(&ptr, page_aligned_mem, num * sizeof(T))) throw std::bad_alloc();

        // madvise is a system call to allocate
        // huge pages. By allocating huge pages
        // for memory allocation improves overall
        // time by reduction in page initialization
        // in next step.
        madvise(ptr, num, MADV_HUGEPAGE);

        T* array = reinterpret_cast<T*>(ptr);

        // Write a value in each virtual memory page to force the
        // materialization in physical memory to avoid paying this price later
        // during the first use of the memory
        for (std::size_t i = 0; i < num; i += (page_aligned_mem)) array[i] = 0;
        return array;
    }

    // Dummy construct which doesnt initialize
    template <typename U>
    void construct(U* ptr) noexcept {
        // Skip the default construction
    }

    void deallocate(T* p, std::size_t num) { free(p); }

    void deallocate(T* p) { free(p); }
};

namespace dataCompression{
namespace internal{
constexpr uint64_t BLOCK_SIZE_IN_KB=1;
constexpr uint64_t BLOCK_SIZE_IN_BYTE=BLOCK_SIZE_IN_KB*1024;
constexpr auto DEFLATE_METHOD = 8;
uint32_t writeGzipHeader(uint8_t* out);
uint32_t writeGzipFooter(uint8_t* out, uint32_t compressSize, uint32_t checksum, const std::string &fileName, uint32_t fileSize);
uint32_t writeZlibHeader(uint8_t *out);
uint32_t writeZlibFooter(uint8_t *out, uint32_t idxAfterCompress, uint32_t checksum);
bool readGzipZlibHeader(uint8_t* in);

uint32_t gzipZlibCompressionInternal(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t &checksum, bool isZlib);
uint32_t gzipZlibCompressionInternal(uint8_t* in, uint8_t* out, uint64_t inputSize, uint64_t maxOutputSize, uint32_t &checksum, bool isZlib);
uint32_t gzipZlibDecompressionInternalMM(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t max_outbuf_size);
uint32_t gzipZlibDecompressionInternalStream(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t max_outbuf_size);
} //internal
uint32_t gzipZlibCompression(uint8_t *in, uint8_t *out, uint32_t inputSize, const std::string &fileName, bool isZlib);
uint32_t gzipZlibDecompression(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t max_output_size, bool stream);
} //dataCompression