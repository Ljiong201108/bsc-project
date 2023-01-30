#include "aes256EcbDec.hpp"

extern "C"{
	void aes256EcbDec(
		ap_uint<128> *cipherTextBuffer, 
		ap_uint<256> *cipherKey, 
		ap_uint<128> *plainTextBuffer, 
		int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer

	ap_uint<256> key_r=cipherKey[0];

	// intermediate registers to perform the decryption chain
	ap_uint<128> ciphertext_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> output_block = 0;
	ap_uint<128> plaintext_r = 0;


decryption_ecb_loop:
	for(int i=0;i<size;i++){
#pragma HLS PIPELINE II = 1

		xf::security::aesDec<256> decipher;
		decipher.updateKey(key_r);
		
		// read a block of ciphertext, 128 bits
		ciphertext_r = cipherTextBuffer[i];

		// calculate input block
		input_block = ciphertext_r;

		// CIPH_k^(-1)
		decipher.process(input_block, key_r, output_block);

		// get the plaintext for current interation by output_block
		plaintext_r = output_block;

		// write out plaintext
		plainTextBuffer[i] = plaintext_r;
	}
}
}