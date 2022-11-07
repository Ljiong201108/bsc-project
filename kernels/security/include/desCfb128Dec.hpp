#pragma once

#include <ap_int.h>
#include "hls_stream.h"
#include "cfb.hpp"
#include "toStream.hpp"
#include "fromStream.hpp"

extern "C"{
    void desCfb128Dec(ap_uint<64> *cipherTextBuffer, ap_uint<64> cipherKey, ap_uint<64> initVec, ap_uint<64> *plainTextBuffer, int size);
}