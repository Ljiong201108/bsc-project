#pragma once

#include "Workshop.hpp"
#include "KernelExecutor.hpp"
#include "StructureAnalyzer.hpp"

struct GzipZlibCompressionItem{
    uint64_t idx;
    uint32_t size;
    uint8_t *payload;
};

class GzipZlibCompressionWorkshop;

class GzipZlibCompressionKernelExecutor : KernelExecutor<GzipZlibCompressionItem>{
protected:
    GzipZlibCompressionWorkshop &workshop;
    bool isZlib;

public:
    uint64_t checksum;

    GzipZlibCompressionKernelExecutor(
        int idx,
        bool isZlib,
        ThreadSafeQueue<GzipZlibCompressionItem> &inputQueue, 
        GeneralQueue &outputQueue, GzipZlibCompressionWorkshop &workshop);

    void process() override;
};

class GzipZlibCompressionStructureAnalyzer : StructureAnalyzer<GzipZlibCompressionItem>{
public:
    GzipZlibCompressionStructureAnalyzer(
        GeneralQueue &inputQueue,
        ThreadSafeQueue<GzipZlibCompressionItem> &outputQueue);
    
    void process() override;
};

class GzipZlibCompressionWorkshop : Workshop<GzipZlibCompressionItem>{
    friend class GzipZlibCompressionKernelExecutor;
    friend class GzipZlibCompressionStructureAnalyzer;
protected:
    const static uint64_t CHUNK_SIZE_IN_KB=64*1024;
    const static uint64_t CHUNK_SIZE_IN_BYTE=CHUNK_SIZE_IN_KB*1024;
    const static uint64_t MCR=2;

    bool isZlib;
    GzipZlibCompressionKernelExecutor kernelExecutor;
    GzipZlibCompressionStructureAnalyzer structureAnalyzer;
    std::thread structureAnalyzerThread;
    std::thread kernelExecutorThread;

public:
    GzipZlibCompressionWorkshop(bool isZlib);
    void wait();
    GeneralQueue& getInputQueue() override;
    GeneralQueue& getOutputQueue() override;
};