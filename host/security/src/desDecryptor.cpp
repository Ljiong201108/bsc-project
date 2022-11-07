#include "desDecryptor.hpp"
#include <vector>

desDecryptor::desDecryptor(Application &app) : m_app(app){}

uint64_t desDecryptor::desSingleBlockDec(const uint64_t &cipher, const uint64_t &key){
    Pool<CommandQueuePointer, 1> commandQueuePool;
    CommandQueuePointer &cq_pointer=commandQueuePool.get<0>();

    Pool<KernelPointer, 1> kernelPool;
    KernelPointer &k_pointer=kernelPool.get<0>();

    Pool<BufferPointer, 1> bufferPool;
    BufferPointer &b_pointer=bufferPool.get<0>();

    cq_pointer.build(m_app.getContext(), m_app.getDevice(), CL_QUEUE_PROFILING_ENABLE, &m_app.getErr());
    k_pointer.build(m_app.getProgram(), "desSingleBlockDec", &m_app.getErr());
    b_pointer.build(m_app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t), (void*)0, &m_app.getErr());

    k_pointer->setArg(0, cipher);
    k_pointer->setArg(1, key);
    k_pointer->setArg(2, *b_pointer);

    uint64_t *out = (uint64_t *)cq_pointer->enqueueMapBuffer(*b_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t));
    cq_pointer->enqueueTask(*k_pointer);
    cq_pointer->enqueueMigrateMemObjects({*b_pointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cq_pointer->finish();
    
    #ifdef DEBUG
    std::cout<<"Plain: "<<std::hex<<out[0]<<std::endl;
    #endif
    return out[0];
}

void desDecryptor::desCbcDec(uint64_t *cipher, const uint64_t &key, const uint64_t &iv, uint64_t *plain, uint32_t size){
    Pool<CommandQueuePointer, 1> commandQueuePool;
    CommandQueuePointer &cq_pointer=commandQueuePool.get<0>().create(m_app.getContext(), m_app.getDevice(), CL_QUEUE_PROFILING_ENABLE);

    Pool<KernelPointer, 1> kernelPool;
    KernelPointer &k_pointer=kernelPool.get<0>().create(m_app.getProgram(), "desCbcDec");

    Pool<BufferPointer, 2> bufferPool;
    BufferPointer &cipher_pointer=bufferPool.get<0>().create(m_app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &plain_pointer=bufferPool.get<1>().create(m_app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

    k_pointer->setArg(0, *cipher_pointer);
    k_pointer->setArg(1, key);
    k_pointer->setArg(2, iv);
    k_pointer->setArg(3, *plain_pointer);
    k_pointer->setArg(4, size);

    uint64_t *cipherMapped = (uint64_t *)cq_pointer->enqueueMapBuffer(*cipher_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t));
    uint64_t *plainMapped = (uint64_t *)cq_pointer->enqueueMapBuffer(*plain_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t));

    for(size_t i=0;i<size;i++) cipherMapped[i]=cipher[i];

    cq_pointer->enqueueMigrateMemObjects({*cipher_pointer}, 0);
    cq_pointer->enqueueTask(*k_pointer);
    cq_pointer->enqueueMigrateMemObjects({*plain_pointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cq_pointer->finish();
    
    for(size_t i=0;i<size;i++) plain[i]=plainMapped[i];
    #ifdef DEBUG
    for(int i=0;i<size;i++) std::cout<<"Cipher "<<i<<": "<<std::fixed<<std::hex<<cipher[i]<<std::endl;
    #endif
}
