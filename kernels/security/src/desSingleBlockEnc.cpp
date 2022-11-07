#include "desSingleBlockEnc.hpp"

extern "C"{

  void desSingleBlockEnc(ap_uint<64> in, ap_uint<64> key, ap_uint<64> out[1]){
    xf::security::desEncrypt(in, key, out[0]);
  }

}