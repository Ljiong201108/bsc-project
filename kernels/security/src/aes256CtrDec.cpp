#include "aes256CtrDec.hpp"

extern "C"{
void aes256CtrDec(
	ap_uint<128> *cipherTextBuffer, 
	ap_uint<256> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *plainTextBuffer, 
	int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer

	ap_uint<256> key_r=cipherKey[0];
	ap_uint<128> iv=initVec[0];

	// intermediate registers to perform the decryption chain
	ap_uint<128> ciphertext_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> input_block_r = 0;
	ap_uint<128> output_block = 0;
	ap_uint<128> plaintext_r = 0;

	// set the initialization for ture
	bool initialization = true;

decryption_ctr_loop:
	for(int i=0;i<size;i++){
#pragma HLS PIPELINE II = 1

		xf::security::aesEnc<256> cipher;
		cipher.updateKey(key_r);

		// read a block of ciphertext, 128 bits
		ciphertext_r = cipherTextBuffer[i];

		// calculate input_block
		if (initialization) {
			input_block = iv;
			initialization = false;
		} else {
			input_block_r.range(127, 120) = input_block(7, 0);
			input_block_r.range(119, 112) = input_block(15, 8);
			input_block_r.range(111, 104) = input_block(23, 16);
			input_block_r.range(103, 96) = input_block(31, 24);
			input_block_r.range(95, 88) = input_block(39, 32);
			input_block_r.range(87, 80) = input_block(47, 40);
			input_block_r.range(79, 72) = input_block(55, 48);
			input_block_r.range(71, 64) = input_block(63, 56);
			input_block_r.range(63, 56) = input_block(71, 64);
			input_block_r.range(55, 48) = input_block(79, 72);
			input_block_r.range(47, 40) = input_block(87, 80);
			input_block_r.range(39, 32) = input_block(95, 88);
			input_block_r.range(31, 24) = input_block(103, 96);
			input_block_r.range(23, 16) = input_block(111, 104);
			input_block_r.range(15, 8) = input_block(119, 112);
			input_block_r.range(7, 0) = input_block(127, 120);
			++input_block_r;
			input_block.range(127, 120) = input_block_r(7, 0);
			input_block.range(119, 112) = input_block_r(15, 8);
			input_block.range(111, 104) = input_block_r(23, 16);
			input_block.range(103, 96) = input_block_r(31, 24);
			input_block.range(95, 88) = input_block_r(39, 32);
			input_block.range(87, 80) = input_block_r(47, 40);
			input_block.range(79, 72) = input_block_r(55, 48);
			input_block.range(71, 64) = input_block_r(63, 56);
			input_block.range(63, 56) = input_block_r(71, 64);
			input_block.range(55, 48) = input_block_r(79, 72);
			input_block.range(47, 40) = input_block_r(87, 80);
			input_block.range(39, 32) = input_block_r(95, 88);
			input_block.range(31, 24) = input_block_r(103, 96);
			input_block.range(23, 16) = input_block_r(111, 104);
			input_block.range(15, 8) = input_block_r(119, 112);
			input_block.range(7, 0) = input_block_r(127, 120);
		}

		// CIPH_k
		cipher.process(input_block, key_r, output_block);

		// get the plaintext for current interation by output_block and ciphertext
		plaintext_r = ciphertext_r ^ output_block;

		// write out plaintext
		plainTextBuffer[i] = plaintext_r;
	}
}
}