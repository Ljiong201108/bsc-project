#include <ap_int.h>
#include "hls_stream.h"
#include "cfb.hpp"
#include "toStream.hpp"
#include "fromStream.hpp"

extern "C"{
    void aes192Cfb128Enc(
        ap_uint<128> *plainTextBuffer, 
        ap_uint<192> *cipherKey, 
        ap_uint<128> *initVec, 
        ap_uint<128> *cipherTextBuffer, 
        int size);
}