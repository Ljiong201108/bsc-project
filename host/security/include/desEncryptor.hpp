#include "Application.hpp"
#include "Pool.hpp"
#include "CommandQueuePointer.hpp"
#include "KernelPointer.hpp"
#include "BufferPointer.hpp"

class desEncryptor{
private:
    Application &m_app;

public:
    desEncryptor(Application &app);
    uint64_t desSingleBlockEnc(const uint64_t &plainText, const uint64_t &key);
    void desCbcEnc(uint64_t *plain, const uint64_t &key, const uint64_t &iv, uint64_t *cipher, uint32_t size);
};