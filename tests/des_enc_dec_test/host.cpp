#include "desEncryptor.hpp"
#include "desDecryptor.hpp"

int main(int argc, char **argv)
{
    Application app("test.xclbin");
    desEncryptor enc(app);
    desDecryptor dec(app);

    std::cout<<"Starting encrypt single block!"<<std::endl;
    uint64_t cipher64=enc.desSingleBlockEnc(0x12345678, 0x66666666);
    std::cout<<"Cipher: "<<std::hex<<cipher64<<std::endl;
    std::cout<<"Finished encrypting successfully!"<<std::endl<<std::endl;

    std::cout<<"Starting decrypt single block!"<<std::endl;
    uint64_t plain64=dec.desSingleBlockDec(cipher64, 0x66666666);
    std::cout<<"Plain: "<<std::hex<<plain64<<std::endl;
    std::cout<<"Finished decrypting successfully!"<<std::endl<<std::endl;
    
    std::cout<<"Starting encrypting CBC!"<<std::endl;
    uint64_t plain[4]={0x1234, 0x2345, 0x3456, 0x4567}, cipher[4];
    uint64_t key=0x66666666, iv=0x11111111;
    enc.desCbcEnc(plain, key, iv, cipher, 4);
    for(int i=0;i<4;i++) std::cout<<"Cipher "<<i<<": "<<std::hex<<cipher[i]<<std::endl;
    std::cout<<"Finished testing successfully!"<<std::endl;

    std::cout<<"Starting decrypting CBC!"<<std::endl;
    uint64_t newPlain[4];
    dec.desCbcDec(cipher, key, iv, newPlain, 4);
    for(int i=0;i<4;i++) std::cout<<"Plain "<<i<<": "<<std::hex<<newPlain[i]<<std::endl;
    std::cout<<"Finished decrypting successfully!"<<std::endl;

    return 0;
}