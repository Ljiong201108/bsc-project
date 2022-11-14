#include "Des.hpp"

namespace testDes{

uint64_t key=0x66666666, iv=0x11111111;

inline void testDesCbc(){
    std::cout<<"Starting testing desCbc!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    
    des::desCbc<Type::ENC>(plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desCbc<Type::DEC>(cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void testDesCfb1(){
    std::cout<<"Starting testing desCfb1!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    
    des::desCfb1<Type::ENC>(plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desCfb1<Type::DEC>(cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void testDesCfb8(){
    std::cout<<"Starting testing desCfb8!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    
    des::desCfb8<Type::ENC>(plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desCfb8<Type::DEC>(cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void testDesCfb128(){
    std::cout<<"Starting testing desCfb128!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    
    des::desCfb128<Type::ENC>(plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desCfb128<Type::DEC>(cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void testDesEcb(){
    std::cout<<"Starting testing desEcb!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    uint64_t key=0x66666666;
    des::desEcb<Type::ENC>(plain, key, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desEcb<Type::DEC>(cipher, key, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void testDesOfb(){
    std::cout<<"Starting testing desOfb!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    
    des::desOfb<Type::ENC>(plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desOfb<Type::DEC>(cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}
}