#include "Aes.hpp"

namespace testAes{

uint64_t key[]={0x66666666, 0x77777777}, iv[]={0x11111111, 0x22222222};

inline void testAes128CbcEnc(){
    std::cout<<"Starting testing aes128CbcEnc!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    
    aes::aesCbc<Type::ENC, aes::KeyLength::U128>(plain, key, iv, cipher, 2);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void testAes128CbcDec(){
    std::cout<<"Starting testing aes128CbcDec!"<<std::endl;

    uint64_t cipher[4]={0x64c72ec93ec821bb, 0xad30d791c12265fa, 0x7aabe371b4d4d1ea, 0x8b891f95b33b50c5}, newPlain[4];
    aes::aesCbc<Type::DEC, aes::KeyLength::U128>(cipher, key, iv, newPlain, 2);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

/*
    Cipher 0: 3bb758a3a7d440b9
    Cipher 1: c078ea81dd472387
    Cipher 2: b147999cced4c140
    Cipher 3: a074feb12365996d
    Tag: 0: ed84c51e60668ea
    Tag: 1: 9358bf6b9a63a6e2
*/
inline void testAes128CcmEnc(){
    std::cout<<"Starting testing aes128CcmEnc!"<<std::endl;

    uint64_t in[4]={0x1111222233334444, 0x2222333344445555, 0x3333444455556666, 0x4444555566667777};
    uint64_t out[4];
    uint64_t ad[2]={0x9999999999999999, 0x8888888888888888};
    uint64_t tag[2];
    uint64_t numBlockIn[1]={2};
    uint64_t numBlockAD[1]={1};
    uint64_t nonce=0x0034567890ABCDEF;
    
    aes::aesCcm<Type::ENC, aes::KeyLength::U128>(in, key, nonce, ad, out, tag, numBlockIn, numBlockAD, 1);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<out[i]<<std::endl;
    for(int i=0;i<2;i++) std::cout<<"Tag "<<i<<": "<<std::hex<<tag[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

/*
    Plain 0: 1111222233334444
    Plain 1: 2222333344445555
    Plain 2: 3333444455556666
    Plain 3: 4444555566667777
    Tag 0: ed84c51e60668ea
    Tag 1: 9358bf6b9a63a6e2
*/
inline void testAes128CcmDec(){
    std::cout<<"Starting testing aes128CcmEnc!"<<std::endl;

    uint64_t in[4]={0x3bb758a3a7d440b9, 0xc078ea81dd472387, 0xb147999cced4c140, 0xa074feb12365996d};
    uint64_t out[4];
    uint64_t ad[2]={0x9999999999999999, 0x8888888888888888};
    uint64_t tag[2];
    uint64_t numBlockIn[1]={2};
    uint64_t numBlockAD[1]={1};
    uint64_t nonce=0x0034567890ABCDEF;
    
    aes::aesCcm<Type::DEC, aes::KeyLength::U128>(in, key, nonce, ad, out, tag, numBlockIn, numBlockAD, 1);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<out[i]<<std::endl;
    for(int i=0;i<2;i++) std::cout<<"Tag "<<i<<": "<<std::hex<<tag[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

/*
    Cipher 0: d985a8df8a9ac90e
    Cipher 1: 4b3eeb3b2a2a532d
    Cipher 2: f141b6600b0efecb
    Cipher 3: 992e5710609c6865
    Tag 0: 888677e3343bc461
    Tag 1: cf1ec55cceeb9ad1
*/
inline void testAes128GcmEnc(){
    std::cout<<"Starting testing aes128GcmEnc!"<<std::endl;

    uint64_t in[4]={0x1111222233334444, 0x2222333344445555, 0x3333444455556666, 0x4444555566667777};
    uint64_t out[4];
    uint64_t ad[2]={0x9999999999999999, 0x8888888888888888};
    uint64_t tag[2];
    uint64_t numBlockIn[1]={2};
    uint64_t numBlockAD[1]={1};
    uint64_t IV[2]={0x1234567890ABCDEF, 0x1234567890ABCDEF};
    
    aes::aesGcm<Type::ENC, aes::KeyLength::U128>(in, key, IV, ad, out, tag, numBlockIn, numBlockAD, 1);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<out[i]<<std::endl;
    for(int i=0;i<2;i++) std::cout<<"Tag "<<i<<": "<<std::hex<<tag[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

/*
    Plain 0: 1111222233334444
    Plain 1: 2222333344445555
    Plain 2: 3333444455556666
    Plain 3: 4444555566667777
    Tag 0: 888677e3343bc461
    Tag 1: cf1ec55cceeb9ad1
*/
inline void testAes128GcmDec(){
    std::cout<<"Starting testing aes128GcmEnc!"<<std::endl;

    uint64_t in[4]={0xd985a8df8a9ac90e, 0x4b3eeb3b2a2a532d, 0xf141b6600b0efecb, 0x992e5710609c6865};
    uint64_t out[4];
    uint64_t ad[2]={0x9999999999999999, 0x8888888888888888};
    uint64_t tag[2];
    uint64_t numBlockIn[1]={2};
    uint64_t numBlockAD[1]={1};
    uint64_t IV[2]={0x1234567890ABCDEF, 0x1234567890ABCDEF};
    
    aes::aesGcm<Type::DEC, aes::KeyLength::U128>(in, key, IV, ad, out, tag, numBlockIn, numBlockAD, 1);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<out[i]<<std::endl;
    for(int i=0;i<2;i++) std::cout<<"Tag "<<i<<": "<<std::hex<<tag[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}
}