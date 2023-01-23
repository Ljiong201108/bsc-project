#pragma once

#include "Aes.hpp"

namespace security{

namespace aes{
void aesCbcEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCbcEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

void aesCbcDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCbcDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

//-------

void aesCfb1Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCfb1Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

void aesCfb1Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCfb1Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

//-------

void aesCfb8Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCfb8Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

void aesCfb8Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCfb8Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

//-------

void aesCfb128Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCfb128Encrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

void aesCfb128Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCfb128Decrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

//-------

void aesEcbEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

void aesEcbDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

//-------

void aesOfbEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesOfbEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

void aesOfbDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesOfbDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

//-------

void aesCtrEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCtrEncrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

void aesCtrDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength, void* iv);
void aesCtrDecrypt(void *src, void* dest, uint32_t size, void *key, uint32_t keyLength);

} // end aes
} // end security