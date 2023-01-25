#pragma once
#include "security.hpp"
#include "Application.hpp"
#include "Pool.hpp"
#include "CommandQueuePointer.hpp"
#include "KernelPointer.hpp"
#include "BufferPointer.hpp"
#include "Timer.hpp"

// #ifdef PERF_MEASURE
#include <chrono>
// #endif

namespace aesHost{

enum class KeyLength{
	U128, U192, U256
};

enum class Method{
	Cbc, Cfb1, Cfb8, Cfb128, Ofb, Ecb, Ccm, Gcm
};

std::string methodToString(Method m);

int keyLengthToNumBits(KeyLength k);

std::string keyLengthToString(KeyLength k);

constexpr Lib keyLengthToLib(KeyLength k);

template<Type T, KeyLength K>
void aesCbc(uint64_t *in, uint64_t *key, uint64_t *iv, uint64_t *out, uint32_t size){
	CommandQueuePointer cqPointer;
	KernelPointer kPointer;
	Pool<BufferPointer, 4> bufferPool;
	const size_t numULL=keyLengthToNumBits(K)/64;

	cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
	kPointer.create(Application::getProgram<Lib::AES128_CBC>(), "aes"+keyLengthToString(K)+"Cbc"+typeToString(T));
	BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &outPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &keyPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
	BufferPointer &ivPointer=bufferPool.get<3>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

	kPointer->setArg(0, *inPointer);
	kPointer->setArg(1, *keyPointer);
	kPointer->setArg(2, *ivPointer);
	kPointer->setArg(3, *outPointer);
	kPointer->setArg(4, size);

	uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);
	uint64_t *ivMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*ivPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);

	for(size_t i=0;i<size*numULL;i++) inMapped[i]=in[i];
	for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];
	for(size_t i=0;i<numULL;i++) ivMapped[i]=iv[i];

	cqPointer->enqueueMigrateMemObjects({*inPointer, *keyPointer, *ivPointer}, 0);
	cqPointer->finish();

// #ifdef PERF_MEASURE
	Timer::startComputeTimer();
// #endif
	cqPointer->enqueueTask(*kPointer);
	cqPointer->finish();
// #ifdef PERF_MEASURE
	Timer::endComputeTimer();
// #endif

	cqPointer->enqueueMigrateMemObjects({*outPointer, *ivPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
	cqPointer->finish();
	
	for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
}

template<Type T, KeyLength K>
void aesCfb1(uint64_t *in, uint64_t *key, uint64_t *iv, uint64_t *out, uint32_t size){
	CommandQueuePointer cqPointer;
	KernelPointer kPointer;
	Pool<BufferPointer, 4> bufferPool;
	const size_t numULL=keyLengthToNumBits(K)/64;

	cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
	kPointer.create(Application::getProgram<Lib::AES128_CFB1>(), "aes"+keyLengthToString(K)+"Cfb1"+typeToString(T));
	BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &outPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &keyPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
	BufferPointer &ivPointer=bufferPool.get<3>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

	kPointer->setArg(0, *inPointer);
	kPointer->setArg(1, *keyPointer);
	kPointer->setArg(2, *ivPointer);
	kPointer->setArg(3, *outPointer);
	kPointer->setArg(4, size);

	uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);
	uint64_t *ivMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*ivPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);

	for(size_t i=0;i<size*numULL;i++) inMapped[i]=in[i];
	for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];
	for(size_t i=0;i<numULL;i++) ivMapped[i]=iv[i];

	cqPointer->enqueueMigrateMemObjects({*inPointer, *keyPointer, *ivPointer}, 0);
	cqPointer->finish();

// #ifdef PERF_MEASURE
	Timer::startComputeTimer();
// #endif
	cqPointer->enqueueTask(*kPointer);
	cqPointer->finish();
// #ifdef PERF_MEASURE
	Timer::endComputeTimer();
// #endif

	cqPointer->enqueueMigrateMemObjects({*outPointer, *ivPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
	cqPointer->finish();
	
	for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
}

template<Type T, KeyLength K>
void aesCfb8(uint64_t *in, uint64_t *key, uint64_t *iv, uint64_t *out, uint32_t size){
	CommandQueuePointer cqPointer;
	KernelPointer kPointer;
	Pool<BufferPointer, 4> bufferPool;
	const size_t numULL=keyLengthToNumBits(K)/64;

	cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
	kPointer.create(Application::getProgram<Lib::AES128_CFB8>(), "aes"+keyLengthToString(K)+"Cfb8"+typeToString(T));
	BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &outPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &keyPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
	BufferPointer &ivPointer=bufferPool.get<3>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

	kPointer->setArg(0, *inPointer);
	kPointer->setArg(1, *keyPointer);
	kPointer->setArg(2, *ivPointer);
	kPointer->setArg(3, *outPointer);
	kPointer->setArg(4, size);

	uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);
	uint64_t *ivMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*ivPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);

	for(size_t i=0;i<size*numULL;i++) inMapped[i]=in[i];
	for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];
	for(size_t i=0;i<numULL;i++) ivMapped[i]=iv[i];

	cqPointer->enqueueMigrateMemObjects({*inPointer, *keyPointer, *ivPointer}, 0);
	cqPointer->finish();

// #ifdef PERF_MEASURE
	Timer::startComputeTimer();
// #endif
	cqPointer->enqueueTask(*kPointer);
	cqPointer->finish();
// #ifdef PERF_MEASURE
	Timer::endComputeTimer();
// #endif

	cqPointer->enqueueMigrateMemObjects({*outPointer, *ivPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
	cqPointer->finish();
	
	for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
}

template<Type T, KeyLength K>
void aesCfb128(uint64_t *in, uint64_t *key, uint64_t *iv, uint64_t *out, uint32_t size){
	CommandQueuePointer cqPointer;
	KernelPointer kPointer;
	Pool<BufferPointer, 4> bufferPool;
	const size_t numULL=keyLengthToNumBits(K)/64;

	cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
	kPointer.create(Application::getProgram<Lib::AES128_CFB128>(), "aes"+keyLengthToString(K)+"Cfb128"+typeToString(T));
	BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &outPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &keyPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
	BufferPointer &ivPointer=bufferPool.get<3>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

	kPointer->setArg(0, *inPointer);
	kPointer->setArg(1, *keyPointer);
	kPointer->setArg(2, *ivPointer);
	kPointer->setArg(3, *outPointer);
	kPointer->setArg(4, size);

	uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);
	uint64_t *ivMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*ivPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);

	for(size_t i=0;i<size*numULL;i++) inMapped[i]=in[i];
	for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];
	for(size_t i=0;i<numULL;i++) ivMapped[i]=iv[i];

	cqPointer->enqueueMigrateMemObjects({*inPointer, *keyPointer, *ivPointer}, 0);
	cqPointer->finish();

// #ifdef PERF_MEASURE
	Timer::startComputeTimer();
// #endif
	cqPointer->enqueueTask(*kPointer);
	cqPointer->finish();
// #ifdef PERF_MEASURE
	Timer::endComputeTimer();
// #endif

	cqPointer->enqueueMigrateMemObjects({*outPointer, *ivPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
	cqPointer->finish();
	
	for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
}

template<Type T, KeyLength K>
void aesEcb(uint64_t *in, uint64_t *key, uint64_t *out, uint32_t size){
	CommandQueuePointer cqPointer;
	KernelPointer kPointer;
	Pool<BufferPointer, 3> bufferPool;
	const size_t numULL=keyLengthToNumBits(K)/64;

	cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
	kPointer.create(Application::getProgram<Lib::AES128_ECB>(), "aes"+keyLengthToString(K)+"Ecb"+typeToString(T));
	BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &outPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &keyPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

	kPointer->setArg(0, *inPointer);
	kPointer->setArg(1, *keyPointer);
	kPointer->setArg(2, *outPointer);
	kPointer->setArg(3, size);

	uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);

	for(size_t i=0;i<size*numULL;i++) inMapped[i]=in[i];
	for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];

	cqPointer->enqueueMigrateMemObjects({*inPointer, *keyPointer}, 0);
	cqPointer->finish();

// #ifdef PERF_MEASURE
	Timer::startComputeTimer();
// #endif
	cqPointer->enqueueTask(*kPointer);
	cqPointer->finish();
// #ifdef PERF_MEASURE
	Timer::endComputeTimer();
// #endif

	cqPointer->enqueueMigrateMemObjects({*outPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
	cqPointer->finish();
	
	for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
}

template<Type T, KeyLength K>
void aesOfb(uint64_t *in, uint64_t *key, uint64_t *iv, uint64_t *out, uint32_t size){
	CommandQueuePointer cqPointer;
	KernelPointer kPointer;
	Pool<BufferPointer, 4> bufferPool;
	const size_t numULL=keyLengthToNumBits(K)/64;

	cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
	kPointer.create(Application::getProgram<Lib::AES128_OFB>(), "aes"+keyLengthToString(K)+"Ofb"+typeToString(T));
	BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &outPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &keyPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
	BufferPointer &ivPointer=bufferPool.get<3>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

	kPointer->setArg(0, *inPointer);
	kPointer->setArg(1, *keyPointer);
	kPointer->setArg(2, *ivPointer);
	kPointer->setArg(3, *outPointer);
	kPointer->setArg(4, size);

	uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);
	uint64_t *ivMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*ivPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);

	for(size_t i=0;i<size*numULL;i++) inMapped[i]=in[i];
	for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];
	for(size_t i=0;i<numULL;i++) ivMapped[i]=iv[i];

	cqPointer->enqueueMigrateMemObjects({*inPointer, *keyPointer, *ivPointer}, 0);
	cqPointer->finish();

// #ifdef PERF_MEASURE
	Timer::startComputeTimer();
// #endif
	cqPointer->enqueueTask(*kPointer);
	cqPointer->finish();
// #ifdef PERF_MEASURE
	Timer::endComputeTimer();
// #endif

	cqPointer->enqueueMigrateMemObjects({*outPointer, *ivPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
	cqPointer->finish();
	
	for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
}

template<Type T, KeyLength K>
void aesCtr(uint64_t *in, uint64_t *key, uint64_t *iv, uint64_t *out, uint32_t size){
	CommandQueuePointer cqPointer;
	KernelPointer kPointer;
	Pool<BufferPointer, 4> bufferPool;
	const size_t numULL=keyLengthToNumBits(K)/64;

	cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
	kPointer.create(Application::getProgram<Lib::AES128_CTR>(), "aes"+keyLengthToString(K)+"Ctr"+typeToString(T));
	BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &outPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
	BufferPointer &keyPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
	BufferPointer &ivPointer=bufferPool.get<3>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

	kPointer->setArg(0, *inPointer);
	kPointer->setArg(1, *keyPointer);
	kPointer->setArg(2, *ivPointer);
	kPointer->setArg(3, *outPointer);
	kPointer->setArg(4, size);

	uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
	uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);
	uint64_t *ivMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*ivPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);

	for(size_t i=0;i<size*numULL;i++) inMapped[i]=in[i];
	for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];
	for(size_t i=0;i<numULL;i++) ivMapped[i]=iv[i];

// #ifdef PERF_MEASURE
	Timer::startFPGAIOTimer();
// #endif
	cqPointer->enqueueMigrateMemObjects({*inPointer, *keyPointer, *ivPointer}, 0);
	cqPointer->finish();
// #ifdef PERF_MEASURE
	Timer::endFPGAIOTimer();
// #endif

// #ifdef PERF_MEASURE
	Timer::startComputeTimer();
// #endif
	cqPointer->enqueueTask(*kPointer);
	cqPointer->finish();
// #ifdef PERF_MEASURE
	Timer::endComputeTimer();
// #endif

// #ifdef PERF_MEASURE
	Timer::startFPGAIOTimer();
// #endif
	cqPointer->enqueueMigrateMemObjects({*outPointer, *ivPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
	cqPointer->finish();
// #ifdef PERF_MEASURE
	Timer::endFPGAIOTimer();
// #endif

	for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
}

namespace internal{
// template<Type T, KeyLength K, Method M>
// void aesWithIV(uint64_t *in, uint64_t *key, uint64_t *iv, uint64_t *out, uint32_t size){
//	 CommandQueuePointer cqPointer;
//	 KernelPointer kPointer;
//	 Pool<BufferPointer, 4> bufferPool;
//	 const size_t numULL=keyLengthToNumBits(K)/64;

//	 cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
//	 kPointer.create(Application::getProgram<Lib::AES128_CBC>(), "aes"+keyLengthToString(K)+methodToString(M)+typeToString(T));
//	 BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &outPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &keyPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &ivPointer=bufferPool.get<3>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

//	 kPointer->setArg(0, *inPointer);
//	 kPointer->setArg(1, *keyPointer);
//	 kPointer->setArg(2, *ivPointer);
//	 kPointer->setArg(3, *outPointer);
//	 kPointer->setArg(4, size);

//	 uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
//	 uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
//	 uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);
//	 uint64_t *ivMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*ivPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);

//	 for(size_t i=0;i<size*numULL;i++) inMapped[i]=in[i];
//	 for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];
//	 for(size_t i=0;i<numULL;i++) ivMapped[i]=iv[i];

//	 cqPointer->enqueueMigrateMemObjects({*inPointer, *keyPointer, *ivPointer}, 0);
//	 cqPointer->enqueueTask(*kPointer);
//	 cqPointer->enqueueMigrateMemObjects({*outPointer, *ivPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
//	 cqPointer->finish();
	
//	 for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
// }

// template<Type T, KeyLength K, Method M>
// void aesWithoutIV(uint64_t *in, uint64_t *key, uint64_t *out, uint32_t size){
//	 CommandQueuePointer cqPointer;
//	 KernelPointer kPointer;
//	 Pool<BufferPointer, 3> bufferPool;
//	 const size_t numULL=keyLengthToNumBits(K)/64;

//	 cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
//	 kPointer.create(Application::getProgram(), "aes"+keyLengthToString(K)+methodToString(M)+typeToString(T));
//	 BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &outPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &keyPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

//	 kPointer->setArg(0, *inPointer);
//	 kPointer->setArg(1, *keyPointer);
//	 kPointer->setArg(2, *outPointer);
//	 kPointer->setArg(3, size);

//	 uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
//	 uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*numULL);
//	 uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);

//	 for(size_t i=0;i<size*numULL;i++) inMapped[i]=in[i];
//	 for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];

//	 cqPointer->enqueueMigrateMemObjects({*inPointer, *keyPointer}, 0);
//	 cqPointer->enqueueTask(*kPointer);
//	 cqPointer->enqueueMigrateMemObjects({*outPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
//	 cqPointer->finish();
	
//	 for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
// }

// template<Type T, KeyLength K>
// void aesCcm(uint64_t *in, uint64_t *key, uint64_t nonce, uint64_t *ad, uint64_t *out, uint64_t *tag, uint64_t *numBlockIn, uint64_t *numBlockAD, uint64_t numMessage){
//	 Application &app=Application::getInstance<keyLengthToLib(K)>();
//	 CommandQueuePointer cqPointer;
//	 KernelPointer kPointer;
//	 Pool<BufferPointer, 8> bufferPool;
//	 const size_t numULL=keyLengthToNumBits(K)/64;

//	 cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
//	 kPointer.create(Application::getProgram(), "aes"+keyLengthToString(K)+"Ccm"+typeToString(T));

//	 uint64_t sumBlockIn=0, sumBlockAD=0;
//	 for(uint64_t i=0;i<numMessage;i++) sumBlockIn+=numBlockIn[i], sumBlockAD+=numBlockAD[i];

//	 BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, sumBlockIn*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &adPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, sumBlockAD*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &outPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sumBlockIn*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &keyPointer=bufferPool.get<3>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &tagPointer=bufferPool.get<4>().create(Application::getContext(), CL_MEM_READ_WRITE, numMessage*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &numBlockInPointer=bufferPool.get<5>().create(Application::getContext(), CL_MEM_READ_WRITE, numMessage*sizeof(uint64_t), NULL);
//	 BufferPointer &numBlockADPointer=bufferPool.get<6>().create(Application::getContext(), CL_MEM_READ_WRITE, numMessage*sizeof(uint64_t), NULL);
//	 BufferPointer &noncePointer=bufferPool.get<7>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t), NULL);

//	 kPointer->setArg(0, *inPointer);
//	 kPointer->setArg(1, *keyPointer);
//	 kPointer->setArg(2, *noncePointer);
//	 kPointer->setArg(3, *adPointer);
//	 kPointer->setArg(4, *numBlockInPointer);
//	 kPointer->setArg(5, *numBlockADPointer);
//	 kPointer->setArg(6, *outPointer);
//	 kPointer->setArg(7, *tagPointer);
//	 kPointer->setArg(8, numMessage);

//	 uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sumBlockIn*sizeof(uint64_t)*numULL);
//	 uint64_t *adMapped = (uint64_t * )cqPointer->enqueueMapBuffer(*adPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sumBlockAD*sizeof(uint64_t)*numULL);
//	 uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sumBlockIn*sizeof(uint64_t)*numULL);
//	 uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);
//	 uint64_t *tagMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*tagPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, numMessage*sizeof(uint64_t)*numULL);
//	 uint64_t *numBlockInMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*numBlockInPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, numMessage*sizeof(uint64_t));
//	 uint64_t *numBlockADMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*numBlockADPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, numMessage*sizeof(uint64_t));
//	 uint64_t *nonceMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*noncePointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t));

//	 for(size_t i=0;i<sumBlockIn*numULL;i++) inMapped[i]=in[i];
//	 for(size_t i=0;i<sumBlockAD*numULL;i++) adMapped[i]=ad[i];
//	 for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];
//	 for(size_t i=0;i<numMessage;i++) numBlockInMapped[i]=numBlockIn[i];
//	 for(size_t i=0;i<numMessage;i++) numBlockADMapped[i]=numBlockAD[i];
//	 nonceMapped[0]=nonce;

//	 cqPointer->enqueueMigrateMemObjects({*inPointer, *adPointer, *keyPointer, *numBlockInPointer, *numBlockADPointer, *noncePointer}, 0);
//	 cqPointer->enqueueTask(*kPointer);
//	 cqPointer->enqueueMigrateMemObjects({*outPointer, *tagPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
//	 cqPointer->finish();
	
//	 for(size_t i=0;i<sumBlockIn*numULL;i++) out[i]=outMapped[i];
//	 for(size_t i=0;i<numMessage*numULL;i++) tag[i]=tagMapped[i];
// }

// template<Type T, KeyLength K>
// void aesGcm(uint64_t *in, uint64_t *key, uint64_t *IV, uint64_t *ad, uint64_t *out, uint64_t *tag, uint64_t *numBlockIn, uint64_t *numBlockAD, uint64_t numMessage){
//	 Application &app=Application::getInstance<keyLengthToLib(K)>();
//	 CommandQueuePointer cqPointer;
//	 KernelPointer kPointer;
//	 Pool<BufferPointer, 8> bufferPool;
//	 const size_t numULL=keyLengthToNumBits(K)/64;

//	 cqPointer.create(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
//	 kPointer.create(Application::getProgram(), "aes"+keyLengthToString(K)+"Gcm"+typeToString(T));

//	 uint64_t sumBlockIn=0, sumBlockAD=0;
//	 for(uint64_t i=0;i<numMessage;i++) sumBlockIn+=numBlockIn[i], sumBlockAD+=numBlockAD[i];

//	 BufferPointer &inPointer=bufferPool.get<0>().create(Application::getContext(), CL_MEM_READ_WRITE, sumBlockIn*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &adPointer=bufferPool.get<1>().create(Application::getContext(), CL_MEM_READ_WRITE, sumBlockAD*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &outPointer=bufferPool.get<2>().create(Application::getContext(), CL_MEM_READ_WRITE, sumBlockIn*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &keyPointer=bufferPool.get<3>().create(Application::getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &tagPointer=bufferPool.get<4>().create(Application::getContext(), CL_MEM_READ_WRITE, numMessage*sizeof(uint64_t)*numULL, NULL);
//	 BufferPointer &numBlockInPointer=bufferPool.get<5>().create(Application::getContext(), CL_MEM_READ_WRITE, numMessage*sizeof(uint64_t), NULL);
//	 BufferPointer &numBlockADPointer=bufferPool.get<6>().create(Application::getContext(), CL_MEM_READ_WRITE, numMessage*sizeof(uint64_t), NULL);
//	 BufferPointer &ivPointer=bufferPool.get<7>().create(Application::getContext(), CL_MEM_READ_WRITE, 2*sizeof(uint64_t), NULL);

//	 kPointer->setArg(0, *inPointer);
//	 kPointer->setArg(1, *keyPointer);
//	 kPointer->setArg(2, *ivPointer);
//	 kPointer->setArg(3, *adPointer);
//	 kPointer->setArg(4, *numBlockInPointer);
//	 kPointer->setArg(5, *numBlockADPointer);
//	 kPointer->setArg(6, *outPointer);
//	 kPointer->setArg(7, *tagPointer);
//	 kPointer->setArg(8, numMessage);

//	 uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sumBlockIn*sizeof(uint64_t)*numULL);
//	 uint64_t *adMapped = (uint64_t * )cqPointer->enqueueMapBuffer(*adPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sumBlockAD*sizeof(uint64_t)*numULL);
//	 uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sumBlockIn*sizeof(uint64_t)*numULL);
//	 uint64_t *keyMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*keyPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*numULL);
//	 uint64_t *tagMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*tagPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, numMessage*sizeof(uint64_t)*numULL);
//	 uint64_t *numBlockInMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*numBlockInPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, numMessage*sizeof(uint64_t));
//	 uint64_t *numBlockADMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*numBlockADPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, numMessage*sizeof(uint64_t));
//	 uint64_t *ivMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*ivPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, 2*sizeof(uint64_t));

//	 for(size_t i=0;i<sumBlockIn*numULL;i++) inMapped[i]=in[i];
//	 for(size_t i=0;i<sumBlockAD*numULL;i++) adMapped[i]=ad[i];
//	 for(size_t i=0;i<numULL;i++) keyMapped[i]=key[i];
//	 for(size_t i=0;i<numMessage;i++) numBlockInMapped[i]=numBlockIn[i];
//	 for(size_t i=0;i<numMessage;i++) numBlockADMapped[i]=numBlockAD[i];
//	 for(size_t i=0;i<2;i++) ivMapped[i]=IV[i];

//	 cqPointer->enqueueMigrateMemObjects({*inPointer, *adPointer, *keyPointer, *numBlockInPointer, *numBlockADPointer, *ivPointer}, 0);
//	 cqPointer->enqueueTask(*kPointer);
//	 cqPointer->enqueueMigrateMemObjects({*outPointer, *tagPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
//	 cqPointer->finish();
	
//	 for(size_t i=0;i<sumBlockIn*numULL;i++) out[i]=outMapped[i];
//	 for(size_t i=0;i<numMessage*numULL;i++) tag[i]=tagMapped[i];
// }
} //end namespace internal

// template<Type T, KeyLength K>
// void aesCcm(uint64_t *in, uint64_t *key, uint64_t nonce, uint64_t *ad, uint64_t *out, uint64_t *tag, uint64_t *numBlockIn, uint64_t *numBlockAD, uint64_t numMessage){
//	 aesHost::internal::aesCcm<T, K>(in, key, nonce, ad, out, tag, numBlockIn, numBlockAD, numMessage);
// }

// template<Type T, KeyLength K>
// void aesGcm(uint64_t *in, uint64_t *key, uint64_t *IV, uint64_t *ad, uint64_t *out, uint64_t *tag, uint64_t *numBlockIn, uint64_t *numBlockAD, uint64_t numMessage){
//	 aesHost::internal::aesGcm<T, K>(in, key, IV, ad, out, tag, numBlockIn, numBlockAD, numMessage);
// }
} //end namespace aesHost