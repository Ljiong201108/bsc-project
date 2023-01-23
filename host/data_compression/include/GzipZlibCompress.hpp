#pragma once

#include "Workshop.hpp"
#include "HostCommon.hpp"
#include <functional>

class GzipZlibCompressWorkshop : Workshop{
protected:
	const static uint64_t CHUNK_SIZE_IN_KB=64*1024;
	const static uint64_t CHUNK_SIZE_IN_BYTE=CHUNK_SIZE_IN_KB*1024;
	const static uint64_t MCR=2;

	bool isZlib;
	uint32_t checksum;
	std::thread processThread;

public:
	GzipZlibCompressWorkshop(bool isZlib, bool overlapped=false);
	void processContinuous();
	void processOverlapped();
	void wait();
	uint32_t getChecksum();
	ByteStream& getInputStream() override;
	ByteStream& getOutputStream() override;
};