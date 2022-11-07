#include "desSingleBlockDec.hpp"

extern "C"{

  void desSingleBlockDec(ap_uint<64> in, ap_uint<64> key, ap_uint<64> out[1]){
    xf::security::desDecrypt(in, key, out[0]);
  }

}