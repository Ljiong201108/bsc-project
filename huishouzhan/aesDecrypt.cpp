#include "aesDecrypt.hpp"

template<int KEY_LENGTH>
void aesCbcDec(const std::string &binaryFileName, uint64_t *cipher, const uint64_t *key, const uint64_t *iv, uint64_t *plain, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 4> bufferPool;
    const int num64=KEY_LENGTH/64;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "aes"+KEY_LENGTH+"CbcDec");
    BufferPointer &cipher_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*num64, NULL);
    BufferPointer &plain_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t)*num64, NULL);
    BufferPointer &key_pointer=bufferPool.get<2>().create(app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*num64, NULL);
    BufferPointer &iv_pointer=bufferPool.get<3>().create(app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t)*num64, NULL);

    k_pointer->setArg(0, *cipher_pointer);
    k_pointer->setArg(1, *key_pointer);
    k_pointer->setArg(2, *iv_pointer);
    k_pointer->setArg(3, *plain_pointer);
    k_pointer->setArg(4, size);

    uint64_t *cipherMapped = (uint64_t *)cq_pointer->enqueueMapBuffer(*cipher_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*num64);
    uint64_t *plainMapped = (uint64_t *)cq_pointer->enqueueMapBuffer(*plain_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t)*num64);
    uint64_t *keyMapped = (uint64_t *)cq_pointer->enqueueMapBuffer(*key_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*num64);
    uint64_t *ivMapped = (uint64_t *)cq_pointer->enqueueMapBuffer(*iv_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t)*num64);

    for(size_t i=0;i<size*num64;i++) cipherMapped[i]=cipher[i];
    for(size_t i=0;i<num64;i++) keyMapped[i]=key[i];
    for(size_t i=0;i<num64;i++) ivMapped[i]=iv[i];

    cq_pointer->enqueueMigrateMemObjects({*cipher_pointer, *key_pointer, *iv_pointer}, 0);
    cq_pointer->enqueueTask(*k_pointer);
    cq_pointer->enqueueMigrateMemObjects({*plain_pointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cq_pointer->finish();
    
    for(size_t i=0;i<size*num64;i++) plain[i]=plainMapped[i];
    #ifdef DEBUG
    for(int i=0;i<size;i++) std::cout<<"Cipher "<<i<<": "<<std::fixed<<std::hex<<cipher[i]<<std::endl;
    #endif
}