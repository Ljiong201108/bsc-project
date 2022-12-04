#include <ap_int.h>
#include "hls_stream.h"
#include "cbc.hpp"
#include "toStream.hpp"
#include "fromStream.hpp"

extern "C"{
    void aes256CbcEnc(
        ap_uint<128> *plainTextBuffer, 
        ap_uint<256> *cipherKey, 
        ap_uint<128> *initVec, 
        ap_uint<128> *cipherTextBuffer, 
        int size);
}