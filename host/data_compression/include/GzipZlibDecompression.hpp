#pragma once

#include "Workshop.hpp"
#include "KernelExecutor.hpp"
#include "StructureAnalyzer.hpp"

struct GzipZlibDecompressionItem{
	uint64_t idx;
	uint32_t size;
	uint8_t *payload;
	bool last;
};

class GzipZlibDecompressionWorkshop;

class GzipZlibDecompressionKernelExecutor : KernelExecutor<GzipZlibDecompressionItem>{
protected:
	GzipZlibDecompressionWorkshop &workshop;

public:

	GzipZlibDecompressionKernelExecutor(
		int idx,
		Stream<GzipZlibDecompressionItem> &inputStream, 
		ByteStream &outputStream, 
		GzipZlibDecompressionWorkshop &workshop);

	void process() override;
};

class GzipZlibDecompressionStructureAnalyzer : StructureAnalyzer<GzipZlibDecompressionItem>{
public:
	GzipZlibDecompressionStructureAnalyzer(
		ByteStream &inputStream,
		Stream<GzipZlibDecompressionItem> &outputStream);
	
	void process() override;
};

class GzipZlibDecompressionWorkshop : Workshop<GzipZlibDecompressionItem>{
	friend class GzipZlibDecompressionKernelExecutor;
	friend class GzipZlibDecompressionStructureAnalyzer;
protected:
	const static uint64_t CHUNK_SIZE_IN_KB=64*1024;
	const static uint64_t CHUNK_SIZE_IN_BYTE=CHUNK_SIZE_IN_KB*1024;
	const static uint64_t MCR=2;

	GzipZlibDecompressionKernelExecutor kernelExecutor;
	GzipZlibDecompressionStructureAnalyzer structureAnalyzer;
	std::thread structureAnalyzerThread;
	std::thread kernelExecutorThread;

public:
	GzipZlibDecompressionWorkshop();
	void wait();
	ByteStream& getInputStream() override;
	ByteStream& getOutputStream() override;
};