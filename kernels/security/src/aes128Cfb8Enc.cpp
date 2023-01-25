#include "aes128Cfb8Enc.hpp"

extern "C"{
void aes128Cfb8Enc(
	ap_uint<128> *plainTextBuffer, 
	ap_uint<128> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *cipherTextBuffer, 
	int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer 

	ap_uint<128> key_r=cipherKey[0];
	ap_uint<128> iv=initVec[0];

	xf::security::aesEnc<128> cipher;
	cipher.updateKey(key_r);

	// intermediate registers to perform the encryption chain
	bool next_plaintext = true;
	ap_uint<128> plaintext_r = 0;
	ap_uint<128> feedback_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> output_block = 0;
	ap_uint<128> ciphertext_r = 0;
	ap_uint<4> cfb_byte_cnt = 0;

	// set the initialization for ture
	bool initialization = true;

encryption_cfb8_loop:
	for(int i=0;i<size;){
#pragma HLS PIPELINE
		// read a block of plaintext, 128 bits
		if (next_plaintext) { // mode CFB1/CFB8 needs multiple iteration to process one plaintext block
			plaintext_r = plainTextBuffer[i];
		}

		// calculate input_block
		if (initialization) { // first iteration, input_block is IV
			input_block = iv;
			initialization = false;
		} else { // after first iteration, input_blcok is comprised by 120 bits of IV and 8 bits of ciphertext
			input_block = (input_block >> 8) + (feedback_r(7, 0) << 120);
			if (15 == cfb_byte_cnt) {
				cfb_byte_cnt = 0;
			} else {
				++cfb_byte_cnt;
			}
		}

		// CIPH_k
		cipher.process(input_block, key_r, output_block);

		// feedback for the next iteration and get the ciphertext for current interation
		ciphertext_r(cfb_byte_cnt * 8 + 7, cfb_byte_cnt * 8) =
			plaintext_r(cfb_byte_cnt * 8 + 7, cfb_byte_cnt * 8) ^ output_block(7, 0);
		feedback_r(7, 0) = ciphertext_r(cfb_byte_cnt * 8 + 7, cfb_byte_cnt * 8);

		// write out ciphertext and decide whether to read a new plaintext block or not
		next_plaintext = false;
		if (15 == cfb_byte_cnt) {
			cipherTextBuffer[i]=ciphertext_r;
			next_plaintext = true;
		}

		if (next_plaintext) {
			i++;
		}
	}
}
}