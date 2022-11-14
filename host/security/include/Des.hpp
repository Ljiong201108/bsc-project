#pragma once
#include "security.hpp"
#include "Application.hpp"
#include "Pool.hpp"
#include "CommandQueuePointer.hpp"
#include "KernelPointer.hpp"
#include "BufferPointer.hpp"

namespace des{
enum class Method{
    Cbc, Cfb1, Cfb8, Cfb128, Ofb, Ecb
};

std::string methodToString(Method m);

namespace internal{
template<Type T, Method M>
void desWithIV(const std::string &binaryFileName, uint64_t *in, const uint64_t &key, const uint64_t &iv, uint64_t *out, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cqPointer;
    KernelPointer kPointer;
    Pool<BufferPointer, 2> bufferPool;

    cqPointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    kPointer.create(app.getProgram(), "des"+methodToString(M)+typeToString(T));
    BufferPointer &inPointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &outPointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

    kPointer->setArg(0, *inPointer);
    kPointer->setArg(1, key);
    kPointer->setArg(2, iv);
    kPointer->setArg(3, *outPointer);
    kPointer->setArg(4, size);

    uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t));
    uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t));

    for(size_t i=0;i<size;i++) inMapped[i]=in[i];

    cqPointer->enqueueMigrateMemObjects({*inPointer}, 0);
    cqPointer->enqueueTask(*kPointer);
    cqPointer->enqueueMigrateMemObjects({*outPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cqPointer->finish();
    
    for(size_t i=0;i<size;i++) out[i]=outMapped[i];
    #ifdef DEBUG
    for(int i=0;i<size;i++) std::cout<<"Cipher "<<i<<": "<<std::fixed<<std::hex<<out[i]<<std::endl;
    #endif
}

template<Type T, Method M>
void desWithoutIV(const std::string &binaryFileName, uint64_t *in, const uint64_t &key, uint64_t *out, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cqPointer;
    KernelPointer kPointer;
    Pool<BufferPointer, 2> bufferPool;

    cqPointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    kPointer.create(app.getProgram(), "des"+methodToString(M)+typeToString(T));
    BufferPointer &inPointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &outPointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

    kPointer->setArg(0, *inPointer);
    kPointer->setArg(1, key);
    kPointer->setArg(2, *outPointer);
    kPointer->setArg(3, size);

    uint64_t *inMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*inPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t));
    uint64_t *outMapped = (uint64_t *)cqPointer->enqueueMapBuffer(*outPointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t));

    for(size_t i=0;i<size;i++) inMapped[i]=in[i];

    cqPointer->enqueueMigrateMemObjects({*inPointer}, 0);
    cqPointer->enqueueTask(*kPointer);
    cqPointer->enqueueMigrateMemObjects({*outPointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cqPointer->finish();
    
    for(size_t i=0;i<size;i++) out[i]=outMapped[i];
    #ifdef DEBUG
    for(int i=0;i<size;i++) std::cout<<"Cipher "<<i<<": "<<std::fixed<<std::hex<<out[i]<<std::endl;
    #endif
}
} //end namespace internal

//Cbc, Cfb1, Cfb8, Cfb128, Ofb, Ecb
template<Type T>
void desCbc(const std::string &binaryFileName, uint64_t *in, const uint64_t &key, const uint64_t &iv, uint64_t *out, uint32_t size){
    des::internal::desWithIV<T, Method::Cbc>(binaryFileName, in, key, iv, out, size);
}

template<Type T>
void desCfb1(const std::string &binaryFileName, uint64_t *in, const uint64_t &key, const uint64_t &iv, uint64_t *out, uint32_t size){
    des::internal::desWithIV<T, Method::Cfb1>(binaryFileName, in, key, iv, out, size);
}

template<Type T>
void desCfb8(const std::string &binaryFileName, uint64_t *in, const uint64_t &key, const uint64_t &iv, uint64_t *out, uint32_t size){
    des::internal::desWithIV<T, Method::Cfb8>(binaryFileName, in, key, iv, out, size);
}

template<Type T>
void desCfb128(const std::string &binaryFileName, uint64_t *in, const uint64_t &key, const uint64_t &iv, uint64_t *out, uint32_t size){
    des::internal::desWithIV<T, Method::Cfb128>(binaryFileName, in, key, iv, out, size);
}

template<Type T>
void desEcb(const std::string &binaryFileName, uint64_t *in, const uint64_t &key, uint64_t *out, uint32_t size){
    des::internal::desWithoutIV<T, Method::Ecb>(binaryFileName, in, key, out, size);
}

template<Type T>
void desOfb(const std::string &binaryFileName, uint64_t *in, const uint64_t &key, const uint64_t &iv, uint64_t *out, uint32_t size){
    des::internal::desWithIV<T, Method::Ofb>(binaryFileName, in, key, iv, out, size);
}

} //end namespace des