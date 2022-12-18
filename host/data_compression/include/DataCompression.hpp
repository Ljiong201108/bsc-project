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
#include <mutex>
#include <condition_variable>
#include <memory>
#include <iomanip>

#include "HostCommon.hpp"

constexpr uint64_t MCR=20;

inline void hexdump(void *ptr, unsigned int buflen) {
	std::cout<<std::hex<<std::setfill('0');
	unsigned char *buf=(unsigned char*)ptr;
	unsigned int lenPerLine=256;
	unsigned int i, j;
	for (i=0;i<buflen;i+=lenPerLine) {
		std::cout<<std::setw(8)<<i<<" ";
		for (j=0;j<lenPerLine;j++){
			if (i+j<buflen) std::cout<<std::setw(2)<<(unsigned int)(unsigned char)buf[i+j]<<" ";
			else std::cout<<"   ";
		}
		std::cout<<" ";
		for(j=0;j<lenPerLine;j++){
			if (i+j<buflen) std::cout<<(char)(isprint(buf[i+j]) ? buf[i+j] : '.');
		}
		std::cout<<std::endl;
	}
	std::cout<<std::dec;
}