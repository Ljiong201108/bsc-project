#pragma once

#include <ap_int.h>
#include "hls_stream.h"
#include "ecb.hpp"
#include "toStream.hpp"
#include "fromStream.hpp"

extern "C"{
    void desEcbDec(ap_uint<64> *cipherTextBuffer, ap_uint<64> cipherKey, ap_uint<64> *plainTextBuffer, int size);
}