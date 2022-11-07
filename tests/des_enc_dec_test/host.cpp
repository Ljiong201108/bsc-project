#include "desEncryptor.hpp"
#include "desDecryptor.hpp"

int main(int argc, char **argv)
{
    Application app("test.xclbin");
    desEncryptor enc(app);
    desDecryptor dec(app);

    uint64_t key=0x66666666, iv=0x11111111;

    std::cout<<"Starting encrypt single block!"<<std::endl;
    uint64_t cipher64=enc.desSingleBlockEnc(0x12345678, 0x66666666);
    std::cout<<"Cipher: "<<std::hex<<cipher64<<std::endl;
    std::cout<<"Finished encrypting successfully!"<<std::endl<<std::endl;

    std::cout<<"Starting decrypt single block!"<<std::endl;
    uint64_t plain64=dec.desSingleBlockDec(cipher64, 0x66666666);
    std::cout<<"Plain: "<<std::hex<<plain64<<std::endl;
    std::cout<<"Finished decrypting successfully!"<<std::endl<<std::endl;
    
    std::cout<<"Starting encrypting CBC!"<<std::endl;
    uint64_t plain_cbc[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher_cbc[4];
    enc.desCbcEnc(plain_cbc, key, iv, cipher_cbc, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher_cbc[i]<<std::endl;
    std::cout<<"Finished encrypting successfully!"<<std::endl;

    std::cout<<"Starting decrypting CBC!"<<std::endl;
    uint64_t newPlain_cbc[4];
    dec.desCbcDec(cipher_cbc, key, iv, newPlain_cbc, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain_cbc[i]<<std::endl;
    std::cout<<"Finished decrypting successfully!"<<std::endl;

    std::cout<<"Starting encrypting CFB1!"<<std::endl;
    uint64_t plain_cfb1[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher_cfb1[4];
    enc.desCbcEnc(plain_cfb1, key, iv, cipher_cfb1, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher_cfb1[i]<<std::endl;
    std::cout<<"Finished encrypting successfully!"<<std::endl;

    std::cout<<"Starting decrypting CFB1!"<<std::endl;
    uint64_t newPlain_cfb1[4];
    dec.desCbcDec(cipher_cfb1, key, iv, newPlain_cfb1, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain_cfb1[i]<<std::endl;
    std::cout<<"Finished decrypting successfully!"<<std::endl;

    return 0;
}