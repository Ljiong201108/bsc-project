#pragma once

#include <ap_int.h>
#include "hls_stream.h"
#include "ccm.hpp"
#include "toStream.hpp"
#include "fromStream.hpp"

#ifndef __SYNTHESIS__
#include <iostream>
#endif

extern "C"{
void aes128CcmDec(
    ap_uint<128> *inBuffer, 
    ap_uint<128> *cipherKey, 
    ap_uint<56> *nonce,
    ap_uint<128> *ADBuffer, 
    ap_uint<64> *numBlockInPerMessage,
    ap_uint<64> *numBlockADPerMessage,  
    ap_uint<128> *outBuffer, 
    ap_uint<128> *tagBuffer,
    unsigned long long numMessage);
}