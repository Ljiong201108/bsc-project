#include "desSingleBlock3Dec.hpp"

extern "C"{

  void desSingleBlock3Dec(ap_uint<64> in, ap_uint<64> key1, ap_uint<64> key2, ap_uint<64> key3, ap_uint<64> *out){
    xf::security::des3Decrypt(in, key1, key2, key3, *out);
  }

}