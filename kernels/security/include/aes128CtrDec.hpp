#include <ap_int.h>
#include "hls_stream.h"
#include "ctr.hpp"
#include "toStream.hpp"
#include "fromStream.hpp"

extern "C"{
    void aes128CtrDec(
        ap_uint<128> *cipherTextBuffer, 
        ap_uint<128> *cipherKey, 
        ap_uint<128> *initVec, 
        ap_uint<128> *plainTextBuffer, 
        int size);
}