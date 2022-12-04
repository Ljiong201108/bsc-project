#pragma once

#include "Util.hpp"
#include <stdint.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#include <HostCommon.hpp>

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

inline void hexdump(void *ptr, int buflen) {
	unsigned char *buf = (unsigned char*)ptr;
	int i, j;
	for (i=0; i<buflen; i+=16) {
	printf("%06x: ", i);
	for (j=0; j<16; j++) 
		if (i+j < buflen)
		printf("%02x ", buf[i+j]);
		else
		printf("   ");
	printf(" ");
	for (j=0; j<16; j++) 
		if (i+j < buflen)
		printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
	printf("\n");
	}
}