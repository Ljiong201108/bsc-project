#include "desEncrypt.hpp"
#include <vector>

void desSingleBlockEnc(const std::string &binaryFileName, const uint64_t &plainText, const uint64_t &key, uint64_t &cipher){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    BufferPointer b_pointer;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desSingleBlockEnc");
    b_pointer.create(app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t), NULL);

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
    cipher=out[0];
}

void desSingleBlock3Enc(const std::string &binaryFileName, const uint64_t &plainText, const uint64_t &key1, const uint64_t &key2, const uint64_t &key3, uint64_t &cipher){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    BufferPointer b_pointer;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desSingleBlock3Enc");
    b_pointer.create(app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t), NULL);

    k_pointer->setArg(0, plainText);
    k_pointer->setArg(1, key1);
    k_pointer->setArg(2, key2);
    k_pointer->setArg(3, key3);
    k_pointer->setArg(4, *b_pointer);

    uint64_t *out = (uint64_t *)cq_pointer->enqueueMapBuffer(*b_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t));
    cq_pointer->enqueueTask(*k_pointer);
    cq_pointer->enqueueMigrateMemObjects({*b_pointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cq_pointer->finish();
    
    // #ifdef DEBUG
    std::cout<<"Chiper: "<<std::hex<<out[0]<<std::endl;
    // #endif
    cipher=out[0];
}

void desCbcEnc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desCbcEnc");
    BufferPointer &plain_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &cipher_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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

void desCfb1Enc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desCfb1Enc");
    BufferPointer &plain_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &cipher_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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

void desCfb8Enc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desCfb8Enc");
    BufferPointer &plain_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &cipher_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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

void desCfb128Enc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desCfb128Enc");
    BufferPointer &plain_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &cipher_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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

void desEcbEnc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, uint64_t *cipher, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desEcbEnc");
    BufferPointer &plain_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &cipher_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

    k_pointer->setArg(0, *plain_pointer);
    k_pointer->setArg(1, key);
    k_pointer->setArg(2, *cipher_pointer);
    k_pointer->setArg(3, size);

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

void desOfbEnc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desOfbEnc");
    BufferPointer &plain_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &cipher_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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