#pragma once

#include "des.hpp"

extern "C"{

  void desSingleBlock3Dec(ap_uint<64> in, ap_uint<64> key1, ap_uint<64> key2, ap_uint<64> key3, ap_uint<64> *out);

}