#pragma once
#include "security.hpp"
#include "Application.hpp"
#include "Pool.hpp"
#include "CommandQueuePointer.hpp"
#include "KernelPointer.hpp"
#include "BufferPointer.hpp"

namespace aes{
enum class KeyLength{
    U128, U192, U256
};

enum class Method{
    Cbc, Cfb1, Cfb8, Cfb128, Ofb
};

std::string methodToString(Method m);

int keyLengthToNumBits(KeyLength k);

template<Type T, KeyLength K, Method M>
void aesNormal(std::string &binaryFileName, uint64_t *in, uint64_t *key, uint64_t *iv, uint64_t *out, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cqPointer;
    KernelPointer kPointer;
    Pool<BufferPointer, 4> bufferPool;
    const size_t numULL=keyLengthToNumBits(K)/64;

    cqPointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    kPointer.create(app.getProgram(), "aes"+keyLengthToNumBits(K)+methodToString(M)+typeToString(T));
    BufferPointer &inPointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
    BufferPointer &outPointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*numULL, NULL);
    BufferPointer &keyPointer=bufferPool.get<2>().create(app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);
    BufferPointer &ivPointer=bufferPool.get<3>().create(app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*numULL, NULL);

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
    cqPointer->enqueueTask(*kPointer);
    cqPointer->enqueueMigrateMemObjects({*outPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cqPointer->finish();
    
    for(size_t i=0;i<size*numULL;i++) out[i]=outMapped[i];
    #ifdef DEBUG
    for(int i=0;i<size;i++) std::cout<<"Cipher "<<i<<": "<<std::fixed<<std::hex<<out[i]<<std::endl;
    #endif
}
}