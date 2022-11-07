#include "desEncryptor.hpp"
#include <vector>

desEncryptor::desEncryptor(Application &app) : m_app(app){}

uint64_t desEncryptor::desSingleBlockEnc(const uint64_t &plainText, const uint64_t &key){
    // CommandQueueManager q_m(m_app, CL_QUEUE_PROFILING_ENABLE);
    // KernelManager krnl_des_enc_m(m_app, "desSingleBlockEnc");
    Pool<CommandQueuePointer, 1> commandQueuePool;
    CommandQueuePointer &cq_pointer=commandQueuePool.get<0>();

    Pool<KernelPointer, 1> kernelPool;
    KernelPointer &k_pointer=kernelPool.get<0>();

    Pool<BufferPointer, 1> bufferPool;
    BufferPointer &b_pointer=bufferPool.get<0>();

    cq_pointer.build(m_app.getContext(), m_app.getDevice(), CL_QUEUE_PROFILING_ENABLE, &m_app.getErr());
    k_pointer.build(m_app.getProgram(), "desSingleBlockEnc", &m_app.getErr());
    b_pointer.build(m_app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t), (void*)0, &m_app.getErr());

    // cl::Buffer &out_buf=m_app.newBuffer(CL_MEM_READ_WRITE, sizeof(uint64_t), NULL);

    k_pointer->setArg(0, plainText);
    k_pointer->setArg(1, key);
    k_pointer->setArg(2, *b_pointer);

    uint64_t *out = (uint64_t *)cq_pointer->enqueueMapBuffer(*b_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t));
    cq_pointer->enqueueTask(*k_pointer);
    cq_pointer->enqueueMigrateMemObjects({*b_pointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cq_pointer->finish();
    
    // #ifdef DEBUG
    std::cout<<"Chiper: "<<std::hex<<out[0]<<std::endl;
    // #endif
    return out[0];
}

void desEncryptor::desCbcEnc(uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size){
    Pool<CommandQueuePointer, 1> commandQueuePool;
    CommandQueuePointer &cq_pointer=commandQueuePool.get<0>().create(m_app.getContext(), m_app.getDevice(), CL_QUEUE_PROFILING_ENABLE);

    Pool<KernelPointer, 1> kernelPool;
    KernelPointer &k_pointer=kernelPool.get<0>().create(m_app.getProgram(), "desCbcEnc");

    Pool<BufferPointer, 2> bufferPool;
    BufferPointer &plain_pointer=bufferPool.get<0>().create(m_app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &cipher_pointer=bufferPool.get<1>().create(m_app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

    k_pointer->setArg(0, *plain_pointer);
    k_pointer->setArg(1, key);
    k_pointer->setArg(2, iv);
    k_pointer->setArg(3, *cipher_pointer);
    k_pointer->setArg(4, size);

    uint64_t *plainMapped = (uint64_t *)cq_pointer->enqueueMapBuffer(*plain_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t));
    uint64_t *cipherMapped = (uint64_t *)cq_pointer->enqueueMapBuffer(*cipher_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, size*sizeof(uint64_t));

    for(size_t i=0;i<size;i++) plainMapped[i]=plain[i];

    cq_pointer->enqueueMigrateMemObjects({*plain_pointer}, 0);
    cq_pointer->enqueueTask(*k_pointer);
    cq_pointer->enqueueMigrateMemObjects({*cipher_pointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cq_pointer->finish();
    
    for(size_t i=0;i<size;i++) cipher[i]=cipherMapped[i];
    #ifdef DEBUG
    for(int i=0;i<size;i++) std::cout<<"Cipher "<<i<<": "<<std::fixed<<std::hex<<cipher[i]<<std::endl;
    #endif
}

// CommandQueueManager q_m(m_app, CL_QUEUE_PROFILING_ENABLE);
// KernelManager krnl_des_enc_m(m_app, "desSingleBlockEnc");
// KernelManager krnl_des_dec_m(m_app, "desSingleBlockDec");

// // int64_t plain=0x12345678;
// // int64_t key=0x66666666;
// cl::Buffer &out_buf=m_app.newBuffer(CL_MEM_READ_WRITE, sizeof(uint64_t), NULL);

// krnl_des_enc_m.getContent().setArg(0, plainText);
// krnl_des_enc_m.getContent().setArg(1, key);
// krnl_des_enc_m.getContent().setArg(2, out_buf);

// uint64_t *out = (uint64_t *)q_m.getContent().enqueueMapBuffer(out_buf, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t));
// q_m.getContent().enqueueTask(krnl_des_enc_m.getContent());
// q_m.getContent().enqueueMigrateMemObjects({out_buf}, CL_MIGRATE_MEM_OBJECT_HOST);
// q_m.getContent().finish();

// uint64_t chiper=out[0];
// std::cout<<"Chiper: "<<std::fixed<<std::hex<<chiper<<std::endl;

// krnl_des_dec_m.getContent().setArg(0, chiper);
// krnl_des_dec_m.getContent().setArg(1, key);
// krnl_des_dec_m.getContent().setArg(2, out_buf);

// q_m.getContent().enqueueTask(krnl_des_dec_m.getContent());
// q_m.getContent().enqueueMigrateMemObjects({out_buf}, CL_MIGRATE_MEM_OBJECT_HOST);
// q_m.getContent().finish();

// std::cout<<"Result: "<<std::fixed<<std::hex<<out[0]<<std::endl;