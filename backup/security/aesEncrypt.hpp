#include <cstring>

#include "Application.hpp"
#include "Pool.hpp"
#include "CommandQueuePointer.hpp"
#include "KernelPointer.hpp"
#include "BufferPointer.hpp"

template<int KEY_LENGTH>
void aesCbcEnc(const std::string &binaryFileName, uint64_t *plain, const uint64_t *key, const uint64_t *iv, uint64_t *cipher, uint32_t size);