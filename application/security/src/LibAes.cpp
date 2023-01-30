#include "LibAes.hpp"
#include <assert.h>
#include <climits>

namespace security{
namespace aes{

uint64_t IV[2];

void aesCbcEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCbc<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCbc<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCbc<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);
}

void aesCbcEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCbc<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCbc<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCbc<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);
}

void aesCbcDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCbc<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCbc<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCbc<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)src)+size-16, 16);
}

void aesCbcDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCbc<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCbc<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCbc<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)src)+size-16, 16);
}

//-------

void aesCfb1Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb1<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb1<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb1<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);
}

void aesCfb1Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb1<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb1<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb1<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);
}

void aesCfb1Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb1<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb1<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb1<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)src)+size-16, 16);
}

void aesCfb1Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb1<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb1<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb1<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)src)+size-16, 16);
}

//-------

void aesCfb8Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb8<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb8<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb8<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);
}

void aesCfb8Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb8<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb8<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb8<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);
}

void aesCfb8Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb8<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb8<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb8<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)src)+size-16, 16);
}

void aesCfb8Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb8<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb8<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb8<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)src)+size-16, 16);
}

//-------

void aesCfb128Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb128<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb128<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb128<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);
}

void aesCfb128Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb128<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb128<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb128<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);
}

void aesCfb128Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb128<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb128<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb128<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)src)+size-16, 16);
}

void aesCfb128Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCfb128<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCfb128<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCfb128<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)src)+size-16, 16);
}

//-------

void aesEcbEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesEcb<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesEcb<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesEcb<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
}

void aesEcbDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesEcb<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesEcb<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesEcb<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
}

//-------

void aesOfbEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesOfb<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesOfb<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesOfb<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);

	uint64_t temp[2];
	std::memcpy(temp, ((uint8_t*)src)+size-16, 16);
	IV[0]^=temp[0];
	IV[1]^=temp[1];
}

void aesOfbEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesOfb<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesOfb<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesOfb<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);

	uint64_t temp[2];
	std::memcpy(temp, ((uint8_t*)src)+size-16, 16);
	IV[0]^=temp[0];
	IV[1]^=temp[1];
}

void aesOfbDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesOfb<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesOfb<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesOfb<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);

	uint64_t temp[2];
	std::memcpy(temp, ((uint8_t*)src)+size-16, 16);
	IV[0]^=temp[0];
	IV[1]^=temp[1];
}

void aesOfbDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesOfb<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesOfb<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesOfb<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	std::memcpy(IV, ((uint8_t*)dest)+size-16, 16);

	uint64_t temp[2];
	std::memcpy(temp, ((uint8_t*)src)+size-16, 16);
	IV[0]^=temp[0];
	IV[1]^=temp[1];
}

//-------

void aesCtrEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCtr<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCtr<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCtr<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	
	std::memcpy(IV, iv, 16);
	std::reverse((uint8_t*)IV, ((uint8_t*)IV)+16);
	if(ULLONG_MAX-IV[0]>size){
		IV[0]+=size;
		IV[1]++;
	}else IV[0]+=size;
	std::reverse((uint8_t*)IV, ((uint8_t*)IV)+16);
}

void aesCtrEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCtr<Type::ENC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCtr<Type::ENC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCtr<Type::ENC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	
	std::reverse((uint8_t*)IV, ((uint8_t*)IV)+16);
	if(ULLONG_MAX-IV[0]>size){
		IV[0]+=size;
		IV[1]++;
	}else IV[0]+=size;
	std::reverse((uint8_t*)IV, ((uint8_t*)IV)+16);
}

void aesCtrDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCtr<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCtr<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCtr<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, (uint64_t*)iv, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	
	std::memcpy(IV, iv, 16);
	std::reverse((uint8_t*)IV, ((uint8_t*)IV)+16);
	if(ULLONG_MAX-IV[0]>size){
		IV[0]+=size;
		IV[1]++;
	}else IV[0]+=size;
	std::reverse((uint8_t*)IV, ((uint8_t*)IV)+16);
}

void aesCtrDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength){
	assert(size%16==0);
	uint32_t numBlock=size/16;
    
    if(keyLength==16) aesHost::aesCtr<Type::DEC, aesHost::KeyLength::U128>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==24) aesHost::aesCtr<Type::DEC, aesHost::KeyLength::U192>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else if(keyLength==32) aesHost::aesCtr<Type::DEC, aesHost::KeyLength::U256>((uint64_t*)src, (uint64_t*)key, IV, (uint64_t*)dest, numBlock);
	else { std::cerr<<"wrong key length!"<<std::endl; exit(EXIT_FAILURE); }
	
	std::reverse((uint8_t*)IV, ((uint8_t*)IV)+16);
	if(ULLONG_MAX-IV[0]>size){
		IV[0]+=size;
		IV[1]++;
	}else IV[0]+=size;
	std::reverse((uint8_t*)IV, ((uint8_t*)IV)+16);
}

} // end aes
} // end security