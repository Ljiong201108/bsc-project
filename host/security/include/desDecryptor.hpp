#include "Application.hpp"
#include "Pool.hpp"
#include "CommandQueuePointer.hpp"
#include "KernelPointer.hpp"
#include "BufferPointer.hpp"

class desDecryptor{
private:
    Application &m_app;

public:
    desDecryptor(Application &app);
    uint64_t desSingleBlockDec(const uint64_t &cipher, const uint64_t &key);
    void desCbcDec(uint64_t *cipher, const uint64_t &key, const uint64_t &iv, uint64_t *plain, uint32_t size);
};