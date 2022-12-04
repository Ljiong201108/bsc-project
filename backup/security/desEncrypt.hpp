#include <cstring>

#include "Application.hpp"
#include "Pool.hpp"
#include "CommandQueuePointer.hpp"
#include "KernelPointer.hpp"
#include "BufferPointer.hpp"

void desSingleBlockEnc(const std::string &binaryFileName, const uint64_t &plainText, const uint64_t &key, uint64_t &cipher);
void desSingleBlock3Enc(const std::string &binaryFileName, const uint64_t &plainText, const uint64_t &key1, const uint64_t &key2, const uint64_t &key3, uint64_t &cipher);
void desCbcEnc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size);
void desCfb1Enc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size);
void desCfb8Enc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size);
void desCfb128Enc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size);
void desEcbEnc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, uint64_t *cipher, uint32_t size);
void desOfbEnc(const std::string &binaryFileName, uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size);