#include <ap_int.h>
#include "hls_stream.h"
#include "aes.hpp"

extern "C"{
void aes128OfbDec(
	ap_uint<128> *cipherTextBuffer, 
	ap_uint<256> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *plainTextBuffer, 
	int size);
}