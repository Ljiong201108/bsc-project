#pragma once

#include "Workshop.hpp"
#include "HostCommon.hpp"
#include <functional>

class ZstdCompressWorkshop : Workshop{
protected:
	const uint64_t CHUNK_SIZE_IN_KB=16;
	const uint64_t CHUNK_SIZE_IN_BYTE=CHUNK_SIZE_IN_KB*1024;

	std::thread processThread;

public:
	ZstdCompressWorkshop();
	void process();
	void wait();
	ByteStream& getInputStream() override;
	ByteStream& getOutputStream() override;
};