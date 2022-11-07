#pragma once

#include "des.hpp"

extern "C"{

  void desSingleBlockDec(ap_uint<64> in, ap_uint<64> key, ap_uint<64> *out);

}