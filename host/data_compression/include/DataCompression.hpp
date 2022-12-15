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
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include "HostCommon.hpp"

constexpr uint64_t MCR=20;

inline void hexdump(void *ptr, int buflen, std::string filename) {
	FILE *fptr=NULL;
	if(fptr==NULL) fptr=fopen(filename.c_str(), "a");

	unsigned char *buf = (unsigned char*)ptr;
	int i, j;
	for (i=0; i<buflen; i+=256) {
	fprintf(fptr, "%08x: ", i);
	for (j=0; j<256; j++) 
		if (i+j < buflen)
		fprintf(fptr, "%02x ", buf[i+j]);
		else
		fprintf(fptr, "   ");
	fprintf(fptr, " ");
	for (j=0; j<256; j++) 
		if (i+j < buflen)
		fprintf(fptr, "%c", isprint(buf[i+j]) ? buf[i+j] : '.');
	fprintf(fptr, "\n");
	}

	fclose(fptr);
}