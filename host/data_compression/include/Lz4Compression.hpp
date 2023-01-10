#pragma once

#include "Workshop.hpp"
#include "KernelExecutor.hpp"
#include "StructureAnalyzer.hpp"

struct Lz4CompressionItem{
    uint64_t idx;
    uint32_t size;
    uint8_t *payload;
};

class Lz4CompressionWorkshop;

class Lz4CompressionKernelExecutor : KernelExecutor<Lz4CompressionItem>{
protected:
    Lz4CompressionWorkshop &workshop;

public:
    uint32_t checksum;

    Lz4CompressionKernelExecutor(
        int idx,
        Stream<Lz4CompressionItem> &inputStream, 
        ByteStream &outputStream, 
		Lz4CompressionWorkshop &workshop);

    void process() override;
};

class Lz4CompressionStructureAnalyzer : StructureAnalyzer<Lz4CompressionItem>{
public:
    Lz4CompressionStructureAnalyzer(
        ByteStream &inputStream,
        Stream<Lz4CompressionItem> &outputStream);
    
    void process() override;
};

class Lz4CompressionWorkshop : Workshop<Lz4CompressionItem>{
    friend class Lz4CompressionKernelExecutor;
    friend class Lz4CompressionStructureAnalyzer;
protected:
    // Maximum host buffer used to operate per kernel invocation
	const static uint64_t HOST_BUFFER_SIZE = (32 * 1024 * 1024);

	// Default block size
	const static uint64_t BLOCK_SIZE_IN_KB = 64;
	const static uint64_t BLOCK_SIZE_IN_BYTE = BLOCK_SIZE_IN_KB*1024;

	// Maximum number of blocks based on host buffer size
	const static uint64_t MAX_NUMBER_BLOCKS = (HOST_BUFFER_SIZE / BLOCK_SIZE_IN_BYTE);
    const static uint64_t MCR=2;

    Lz4CompressionKernelExecutor kernelExecutor1;
	Lz4CompressionKernelExecutor kernelExecutor2;
    Lz4CompressionStructureAnalyzer structureAnalyzer;
    std::thread structureAnalyzerThread;
    std::thread kernelExecutorThread1;
	std::thread kernelExecutorThread2;

public:
    Lz4CompressionWorkshop();
    void wait();
    ByteStream& getInputStream() override;
    ByteStream& getOutputStream() override;
};