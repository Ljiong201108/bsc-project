#include "Des.hpp"
#include "Aes.hpp"

std::string binaryFilename="test.xclbin";

inline void test_aes128CBC(){
    std::cout<<"Starting testing aes128Cbc!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    uint64_t key[]={0x66666666, 0x77777777}, iv[]={0x11111111, 0x22222222};
    aes::aesNormal<Type::ENC, aes::KeyLength::U128, aes::Method::Cbc>(binaryFilename, plain, key, iv, cipher, 2);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    aes::aesNormal<Type::DEC, aes::KeyLength::U128, aes::Method::Cbc>(binaryFilename, cipher, key, iv, newPlain, 2);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void test_desCbc(){
    std::cout<<"Starting testing desCbc!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    uint64_t key=0x66666666, iv=0x11111111;
    des::desNormal<Type::ENC, des::Method::Cbc>(binaryFilename, plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desNormal<Type::DEC, des::Method::Cbc>(binaryFilename, cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void test_desCfb1(){
    std::cout<<"Starting testing desCfb1!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    uint64_t key=0x66666666, iv=0x11111111;
    des::desNormal<Type::ENC, des::Method::Cfb1>(binaryFilename, plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desNormal<Type::DEC, des::Method::Cfb1>(binaryFilename, cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void test_desCfb8(){
    std::cout<<"Starting testing desCfb8!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    uint64_t key=0x66666666, iv=0x11111111;
    des::desNormal<Type::ENC, des::Method::Cfb8>(binaryFilename, plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desNormal<Type::DEC, des::Method::Cfb8>(binaryFilename, cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void test_desCfb128(){
    std::cout<<"Starting testing desCfb128!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    uint64_t key=0x66666666, iv=0x11111111;
    des::desNormal<Type::ENC, des::Method::Cfb128>(binaryFilename, plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desNormal<Type::DEC, des::Method::Cfb128>(binaryFilename, cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void test_desEcb(){
    std::cout<<"Starting testing desEcb!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    uint64_t key=0x66666666;
    des::desEcb<Type::ENC>(binaryFilename, plain, key, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desEcb<Type::DEC>(binaryFilename, cipher, key, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

inline void test_desOfb(){
    std::cout<<"Starting testing desOfb!"<<std::endl;

    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    uint64_t key=0x66666666, iv=0x11111111;
    des::desNormal<Type::ENC, des::Method::Ofb>(binaryFilename, plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;

    uint64_t newPlain[4];
    des::desNormal<Type::DEC, des::Method::Ofb>(binaryFilename, cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;

    std::cout<<"Finished testing successfully!"<<std::endl;
}

int main(int argc, char **argv){
    test_desCbc();
    test_desCfb1();
    test_desCfb8();
    test_desCfb128();
    test_desEcb();
    test_desOfb();
    test_aes128CBC();
    return 0;
}