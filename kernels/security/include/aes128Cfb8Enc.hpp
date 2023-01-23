#include <ap_int.h>
#include "hls_stream.h"
#include "aes.hpp"

extern "C"{
void aes128Cfb8Enc(
	ap_uint<128> *plainTextBuffer, 
	ap_uint<128> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *cipherTextBuffer, 
	int size);
}