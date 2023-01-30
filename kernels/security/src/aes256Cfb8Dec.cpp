#include "aes256Cfb8Dec.hpp"

extern "C"{
void aes256Cfb8Dec(
	ap_uint<128> *cipherTextBuffer, 
	ap_uint<256> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *plainTextBuffer, 
	int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer

	ap_uint<256> key_r=cipherKey[0];
	ap_uint<128> iv=initVec[0];

	xf::security::aesEnc<256> cipher;
	cipher.updateKey(key_r);

	// intermediate registers to perform the decryption chain
	bool next_ciphertext = true;
	ap_uint<128> ciphertext_r = 0;
	ap_uint<128> plaintext_r = 0;
	ap_uint<128> feedback_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> output_block = 0;
	ap_uint<4> cfb_byte_cnt = 0;

	// set the initialization for ture
	bool initialization = true;

decryption_cfb8_loop:
	for(int i=0;i<size;){
#pragma HLS PIPELINE II = 1
		// read a block of ciphertext, 128 bits
		if (next_ciphertext) { // mode cfb8 needs 16 iterations to process one ciphertext block
			ciphertext_r = cipherTextBuffer[i];
		}

		// calculate input_block
		if (initialization) { // first iteration, input_block is IV
			input_block = iv;
			initialization = false;
		} else { // after first iteration, input_block is calculated by ciphertext and input_block of last iteration
			input_block.range(119, 0) = input_block.range(127, 8);
			input_block.range(127, 120) = feedback_r.range(7, 0);
			if (15 == cfb_byte_cnt) {
				cfb_byte_cnt = 0;
			} else {
				++cfb_byte_cnt;
			}
		}

		// CIPH_k
		cipher.process(input_block, key_r, output_block);

		// feedback for the next iteration and get the plaintext for current interation
		feedback_r(7, 0) = ciphertext_r(cfb_byte_cnt * 8 + 7, cfb_byte_cnt * 8);
		plaintext_r(cfb_byte_cnt * 8 + 7, cfb_byte_cnt * 8) =
			ciphertext_r.range(cfb_byte_cnt * 8 + 7, cfb_byte_cnt * 8) ^ output_block.range(7, 0);

		// write out plaintext
		next_ciphertext = false;
		if (15 == cfb_byte_cnt) {
			plainTextBuffer[i] = plaintext_r;
			next_ciphertext = true;
		}

		if (next_ciphertext) {
			i++;
		}
	}
}
}