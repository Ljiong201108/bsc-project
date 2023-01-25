#include <ap_int.h>
#include "hls_stream.h"
#include "aes.hpp"

extern "C"{
void aes128EcbDec(
	ap_uint<128> *cipherTextBuffer, 
	ap_uint<192> *cipherKey, 
	ap_uint<128> *plainTextBuffer, 
	int size);
}