#include "aes128Cfb128Dec.hpp"

extern "C"{
void aes128Cfb128Dec(
	ap_uint<128> *cipherTextBuffer, 
	ap_uint<128> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *plainTextBuffer, 
	int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer

	ap_uint<128> key_r=cipherKey[0];
	ap_uint<128> iv=initVec[0];

	xf::security::aesEnc<128> cipher;
	cipher.updateKey(key_r);

	// intermediate registers to perform the decryption chain
	ap_uint<128> ciphertext_r = 0;
	ap_uint<128> plaintext_r = 0;
	ap_uint<128> feedback_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> output_block = 0;

	// set the initialization for ture
	bool initialization = true;

decryption_cfb128_loop:
	for(int i=0;i<size;i++){
#pragma HLS PIPELINE II = 1
		// read a block of ciphertext, 128 bits
		ciphertext_r = cipherTextBuffer[i];

		// calculate input_block
		if (initialization) { // first iteration, input_block is IV
			input_block = iv;
			initialization = false;
		} else { // after first iteration, input_block is ciphertext of last iteration
			input_block = feedback_r;
		}

		// CIPH_k
		cipher.process(input_block, key_r, output_block);

		// feedback for the next iteration and get the plaintext for current interation
		feedback_r = ciphertext_r;
		plaintext_r = ciphertext_r ^ output_block;

		// write out plaintext
		plainTextBuffer[i] = plaintext_r;
	}
}
}