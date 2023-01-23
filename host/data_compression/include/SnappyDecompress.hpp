#pragma once

#include "Workshop.hpp"
#include "HostCommon.hpp"
#include <functional>

class SnappyDecompressWorkshop : Workshop{
protected:
	// Maximum host buffer used to operate per kernel invocation
	const uint32_t HOST_BUFFER_SIZE = (8 * 1024 * 1024);

	// Default block size
	const uint32_t BLOCK_SIZE_IN_KB = 64;

	// Maximum number of blocks based on host buffer size
	const uint32_t MAX_NUMBER_BLOCKS = (HOST_BUFFER_SIZE / (BLOCK_SIZE_IN_KB * 1024));

	std::thread processThread;

public:
	SnappyDecompressWorkshop();
	void process();
	void wait();
	ByteStream& getInputStream() override;
	ByteStream& getOutputStream() override;
};