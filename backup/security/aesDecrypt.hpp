#include <cstring>

#include "Application.hpp"
#include "Pool.hpp"
#include "CommandQueuePointer.hpp"
#include "KernelPointer.hpp"
#include "BufferPointer.hpp"

template<int KEY_LENGTH>
void aesCbcDec(const std::string &binaryFileName, uint64_t *cipher, const uint64_t *key, const uint64_t *iv, uint64_t *plain, uint32_t size);