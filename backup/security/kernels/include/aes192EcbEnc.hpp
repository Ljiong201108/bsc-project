#include <ap_int.h>
#include "hls_stream.h"
#include "ecb.hpp"
#include "toStream.hpp"
#include "fromStream.hpp"

extern "C"{
    void aes192CbcEnc(
        ap_uint<128> *plainTextBuffer, 
        ap_uint<192> *cipherKey, 
        ap_uint<128> *cipherTextBuffer, 
        int size);
}