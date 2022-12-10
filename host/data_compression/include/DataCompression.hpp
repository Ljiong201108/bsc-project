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
#include <cassert>
#include <algorithm>

#include "HostCommon.hpp"

constexpr uint64_t MCR=20;

inline void hexdump(void *ptr, int buflen) {
	unsigned char *buf = (unsigned char*)ptr;
	int i, j;
	for (i=0; i<buflen; i+=128) {
	printf("%08x: ", i);
	for (j=0; j<256; j++) 
		if (i+j < buflen)
		printf("%02x ", buf[i+j]);
		else
		printf("   ");
	printf(" ");
	for (j=0; j<256; j++) 
		if (i+j < buflen)
		printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
	printf("\n");
	}
}