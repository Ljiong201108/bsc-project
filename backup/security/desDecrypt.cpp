#include "desDecrypt.hpp"

void desSingleBlockDec(const std::string &binaryFileName, const uint64_t &cipher, const uint64_t &key, uint64_t &plain){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    BufferPointer b_pointer;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desSingleBlockDec");
    b_pointer.create(app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t), NULL);

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
    plain=out[0];
}

void desSingleBlock3Dec(const std::string &binaryFileName, const uint64_t &cipher, const uint64_t &key1, const uint64_t &key2, const uint64_t &key3, uint64_t &plain){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    BufferPointer b_pointer;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desSingleBlock3Dec");
    b_pointer.create(app.getContext(), CL_MEM_READ_WRITE, sizeof(uint64_t), NULL);

    k_pointer->setArg(0, cipher);
    k_pointer->setArg(1, key1);
    k_pointer->setArg(2, key2);
    k_pointer->setArg(3, key3);
    k_pointer->setArg(4, *b_pointer);

    uint64_t *out = (uint64_t *)cq_pointer->enqueueMapBuffer(*b_pointer, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(uint64_t));
    cq_pointer->enqueueTask(*k_pointer);
    cq_pointer->enqueueMigrateMemObjects({*b_pointer}, CL_MIGRATE_MEM_OBJECT_HOST);
    cq_pointer->finish();
    
    #ifdef DEBUG
    std::cout<<"Plain: "<<std::hex<<out[0]<<std::endl;
    #endif
    plain=out[0];
}

void desCbcDec(const std::string &binaryFileName, uint64_t *cipher, const uint64_t &key, const uint64_t &iv, uint64_t *plain, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desCbcDec");
    BufferPointer &cipher_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &plain_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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

void desCfb1Dec(const std::string &binaryFileName, uint64_t *cipher, const uint64_t &key, const uint64_t &iv, uint64_t *plain, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desCfb1Dec");
    BufferPointer &cipher_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &plain_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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

void desCfb8Dec(const std::string &binaryFileName, uint64_t *cipher, const uint64_t &key, const uint64_t &iv, uint64_t *plain, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desCfb8Dec");
    BufferPointer &cipher_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &plain_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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

void desCfb128Dec(const std::string &binaryFileName, uint64_t *cipher, const uint64_t &key, const uint64_t &iv, uint64_t *plain, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desCfb128Dec");
    BufferPointer &cipher_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &plain_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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

void desEcbDec(const std::string &binaryFileName, uint64_t *cipher, const uint64_t &key, uint64_t *plain, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desEcbDec");
    BufferPointer &cipher_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &plain_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

    k_pointer->setArg(0, *cipher_pointer);
    k_pointer->setArg(1, key);
    k_pointer->setArg(2, *plain_pointer);
    k_pointer->setArg(3, size);

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

void desOfbDec(const std::string &binaryFileName, uint64_t *cipher, const uint64_t &key, const uint64_t &iv, uint64_t *plain, uint32_t size){
    Application app(binaryFileName);
    CommandQueuePointer cq_pointer;
    KernelPointer k_pointer;
    Pool<BufferPointer, 2> bufferPool;

    cq_pointer.create(app.getContext(), app.getDevice(), CL_QUEUE_PROFILING_ENABLE);
    k_pointer.create(app.getProgram(), "desOfbDec");
    BufferPointer &cipher_pointer=bufferPool.get<0>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);
    BufferPointer &plain_pointer=bufferPool.get<1>().create(app.getContext(), CL_MEM_READ_WRITE, size*sizeof(uint64_t), NULL);

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