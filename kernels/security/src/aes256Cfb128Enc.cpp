#include "aes256Cfb128Enc.hpp"

extern "C"{
void aes256Cfb128Enc(
	ap_uint<128> *plainTextBuffer, 
	ap_uint<256> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *cipherTextBuffer, 
	int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer 

	ap_uint<256> key_r=cipherKey[0];
	ap_uint<128> iv=initVec[0];

	xf::security::aesEnc<256> cipher;
	cipher.updateKey(key_r);

	// intermediate registers to perform the encryption chain
	ap_uint<128> plaintext_r = 0;
	ap_uint<128> feedback_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> output_block = 0;
	ap_uint<128> ciphertext_r = 0;

	// set the initialization for ture
	bool initialization = true;

encryption_cfb128_loop:
	for(int i=0;i<size;i++){
#pragma HLS PIPELINE
		// read a block of plaintext, 128 bits
		plaintext_r = plainTextBuffer[i];

		// calculate input_block
		if (initialization) { // first iteration, input_block is IV
			input_block = iv;
			initialization = false;
		} else { // after first iteration, input_blcok is ciphertext of last iteration
			input_block = feedback_r;
		}

		// CIPH_k
		cipher.process(input_block, key_r, output_block);

		// feedback for the next iteration and get the ciphertext for current interation
		ciphertext_r = plaintext_r ^ output_block;
		feedback_r = ciphertext_r;

		// write out ciphertext
		cipherTextBuffer[i]=ciphertext_r;
	}
}
}