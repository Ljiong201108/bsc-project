#pragma once

#include "Workshop.hpp"
#include "KernelExecutor.hpp"
#include "StructureAnalyzer.hpp"

struct Lz4DecompressionItem{
	uint64_t idx;
	uint32_t numBlocks;
	uint32_t blockSizeInBytes;
	uint32_t *compressedSize;
	uint8_t *payload;
};

class Lz4DecompressionWorkshop;

class Lz4DecompressionKernelExecutor : KernelExecutor<Lz4DecompressionItem>{
protected:
	Lz4DecompressionWorkshop &workshop;

public:

	Lz4DecompressionKernelExecutor(
		int idx,
		Stream<Lz4DecompressionItem> &inputStream, 
		ByteStream &outputStream, 
		Lz4DecompressionWorkshop &workshop);

	void process() override;
};

class Lz4DecompressionStructureAnalyzer : StructureAnalyzer<Lz4DecompressionItem>{
public:
	Lz4DecompressionStructureAnalyzer(
		ByteStream &inputStream,
		Stream<Lz4DecompressionItem> &outputStream);
	
	void process() override;
};

class Lz4DecompressionWorkshop : Workshop<Lz4DecompressionItem>{
	friend class Lz4DecompressionKernelExecutor;
	friend class Lz4DecompressionStructureAnalyzer;
protected:
	// Maximum host buffer used to operate per kernel invocation
	const static uint64_t HOST_BUFFER_SIZE = (32 * 1024 * 1024);

	// Default block size
	const static uint64_t BLOCK_SIZE_IN_KB = 64;
	const static uint64_t BLOCK_SIZE_IN_BYTE = BLOCK_SIZE_IN_KB*1024;

	// Maximum number of blocks based on host buffer size
	const static uint64_t MAX_NUMBER_BLOCKS = (HOST_BUFFER_SIZE / BLOCK_SIZE_IN_BYTE);
	const static uint64_t MCR=2;

	Lz4DecompressionKernelExecutor kernelExecutor1;
	// Lz4DecompressionKernelExecutor kernelExecutor2;
	// Lz4DecompressionKernelExecutor kernelExecutor3;
	// Lz4DecompressionKernelExecutor kernelExecutor4;
	// Lz4DecompressionKernelExecutor kernelExecutor5;
	// Lz4DecompressionKernelExecutor kernelExecutor6;
	// Lz4DecompressionKernelExecutor kernelExecutor7;
	// Lz4DecompressionKernelExecutor kernelExecutor8;
	Lz4DecompressionStructureAnalyzer structureAnalyzer;
	std::thread structureAnalyzerThread;
	std::thread kernelExecutorThread1;
	// std::thread kernelExecutorThread2;
	// std::thread kernelExecutorThread3;
	// std::thread kernelExecutorThread4;
	// std::thread kernelExecutorThread5;
	// std::thread kernelExecutorThread6;
	// std::thread kernelExecutorThread7;
	// std::thread kernelExecutorThread8;

public:
	Lz4DecompressionWorkshop();
	void wait();
	ByteStream& getInputStream() override;
	ByteStream& getOutputStream() override;
};