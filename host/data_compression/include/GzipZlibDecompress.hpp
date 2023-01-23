#pragma once

#include "Workshop.hpp"
#include "HostCommon.hpp"
#include <functional>

class GzipZlibDecompressWorkshop : Workshop{
protected:
	const static uint64_t CHUNK_SIZE_IN_KB=64*1024;
	const static uint64_t CHUNK_SIZE_IN_BYTE=CHUNK_SIZE_IN_KB*1024;
	const static uint64_t MCR=2;

	std::thread processThread;

public:
	GzipZlibDecompressWorkshop();
	void process();
	void wait();
	ByteStream& getInputStream() override;
	ByteStream& getOutputStream() override;
};