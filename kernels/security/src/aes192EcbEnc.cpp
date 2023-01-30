#include "aes192EcbEnc.hpp"

extern "C"{
void aes192EcbEnc(
	ap_uint<128> *plainTextBuffer, 
	ap_uint<192> *cipherKey, 
	ap_uint<128> *cipherTextBuffer, 
	int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer 

	ap_uint<192> key_r=cipherKey[0];

	xf::security::aesEnc<192> cipher;
	cipher.updateKey(key_r);
	// intermediate registers to perform the encryption chain
	ap_uint<128> plaintext_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> input_block_r = 0;
	ap_uint<128> output_block = 0;
	ap_uint<128> ciphertext_r = 0;

	// set the initialization for ture
	bool initialization = true;

encryption_ecb_loop:
	for(int i=0;i<size;i++){
#pragma HLS PIPELINE II = 1
		// read a block of plaintext, 128 bits
		plaintext_r = plainTextBuffer[i];

		// calculate input_block
		input_block = plaintext_r;

		// CIPH_k
		cipher.process(input_block, key_r, output_block);

		// get the ciphertext for current interation by output_block and plaintext
		ciphertext_r = output_block;

		// write out ciphertext
		cipherTextBuffer[i]=ciphertext_r;
	}
}
}