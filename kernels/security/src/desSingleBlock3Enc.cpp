#include "desSingleBlock3Enc.hpp"

extern "C"{

  void desSingleBlock3Enc(ap_uint<64> in, ap_uint<64> key1, ap_uint<64> key2, ap_uint<64> key3, ap_uint<64> *out){
    xf::security::des3Encrypt(in, key1, key2, key3, *out);
  }

}