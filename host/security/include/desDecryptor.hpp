#include "Application.hpp"
#include "Pool.hpp"
#include "CommandQueuePointer.hpp"
#include "KernelPointer.hpp"
#include "BufferPointer.hpp"

void desSingleBlockDec(const std::string &binaryFileName, const uint64_t &cipher, const uint64_t &key, uint64_t &plain);
void desSingleBlock3Dec(const std::string &binaryFileName, const uint64_t &cipher, const uint64_t &key1, const uint64_t &key2, const uint64_t &key3, uint64_t &plain);
void desCbcDec(const std::string &binaryFileName, uint64_t *cipher, const uint64_t &key, const uint64_t &iv, uint64_t *plain, uint32_t size);