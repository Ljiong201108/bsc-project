#pragma once

#include <ap_int.h>
#include "hls_stream.h"
#include "cbc.hpp"
#include "toStream.hpp"
#include "fromStream.hpp"

extern "C"{
    void desCbcEnc(ap_uint<64> *plainTextBuffer, ap_uint<64> cipherKey, ap_uint<64> initVec, ap_uint<64> *cipherTextBuffer, int size);
}