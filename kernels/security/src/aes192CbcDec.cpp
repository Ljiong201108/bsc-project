#include "aes192CbcDec.hpp"

extern "C"{
void aes192CbcDec(
	ap_uint<128> *cipherTextBuffer, 
	ap_uint<192> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *plainTextBuffer, 
	int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer

	ap_uint<192> key_r=cipherKey[0];
	ap_uint<128> iv=initVec[0];

	ap_uint<128> ciphertext_r = 0;
	ap_uint<128> plaintext_r = 0;
	ap_uint<128> feedback_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> output_block = 0;

	feedback_r = iv;

aes192CbcDec:
	for(int i=0;i<size;i++){
#pragma HLS pipeline II = 1

		// local decipher
		xf::security::aesDec<192> decipher;
		decipher.updateKey(key_r);

		// read a block of ciphertext, 128 bits
		ciphertext_r = cipherTextBuffer[i];

		// calculate input_block
		input_block = ciphertext_r;

		// CIPH_k^(-1)
		decipher.process(input_block, key_r, output_block);

		plaintext_r = output_block ^ feedback_r;
		feedback_r = ciphertext_r;

		// write out plaintext
		plainTextBuffer[i] = plaintext_r;
	}
}
}