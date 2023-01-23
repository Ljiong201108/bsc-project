#include "aes128Cfb1Dec.hpp"

extern "C"{
	void aes128Cfb1Dec(
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
	bool next_ciphertext = true;
	ap_uint<128> ciphertext_r = 0;
	ap_uint<128> plaintext_r = 0;
	ap_uint<128> feedback_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> output_block = 0;
	ap_uint<4> cfb_byte_cnt = 0;
	ap_uint<3> cfb_bit_cnt = 7;

	// set the initialization for ture
	bool initialization = true;

decryption_cfb1_loop:
	for(int i=0;i<size;){
#pragma HLS PIPELINE II = 1
		// read a block of ciphertext, 128 bits
		if (next_ciphertext) { // mode cfb1 needs 128 iterations to process one ciphertext block
			ciphertext_r = cipherTextBuffer[i];
		}

		// calculate input_block
		if (initialization) { // first iteration, input_block is IV
			input_block = iv;
			initialization = false;
		} else { // after first iteration, input_block is calculated by ciphertext and input_block of last iteration
			ap_uint<128> input_block_r;
			input_block_r.range(127, 120) = input_block.range(7, 0);
			input_block_r.range(119, 112) = input_block.range(15, 8);
			input_block_r.range(111, 104) = input_block.range(23, 16);
			input_block_r.range(103, 96) = input_block.range(31, 24);
			input_block_r.range(95, 88) = input_block.range(39, 32);
			input_block_r.range(87, 80) = input_block.range(47, 40);
			input_block_r.range(79, 72) = input_block.range(55, 48);
			input_block_r.range(71, 64) = input_block.range(63, 56);
			input_block_r.range(63, 56) = input_block.range(71, 64);
			input_block_r.range(55, 48) = input_block.range(79, 72);
			input_block_r.range(47, 40) = input_block.range(87, 80);
			input_block_r.range(39, 32) = input_block.range(95, 88);
			input_block_r.range(31, 24) = input_block.range(103, 96);
			input_block_r.range(23, 16) = input_block.range(111, 104);
			input_block_r.range(15, 8) = input_block.range(119, 112);
			input_block_r.range(7, 0) = input_block.range(127, 120);
			input_block_r = (input_block_r << 1) + feedback_r[120];
			input_block.range(127, 120) = input_block_r.range(7, 0);
			input_block.range(119, 112) = input_block_r.range(15, 8);
			input_block.range(111, 104) = input_block_r.range(23, 16);
			input_block.range(103, 96) = input_block_r.range(31, 24);
			input_block.range(95, 88) = input_block_r.range(39, 32);
			input_block.range(87, 80) = input_block_r.range(47, 40);
			input_block.range(79, 72) = input_block_r.range(55, 48);
			input_block.range(71, 64) = input_block_r.range(63, 56);
			input_block.range(63, 56) = input_block_r.range(71, 64);
			input_block.range(55, 48) = input_block_r.range(79, 72);
			input_block.range(47, 40) = input_block_r.range(87, 80);
			input_block.range(39, 32) = input_block_r.range(95, 88);
			input_block.range(31, 24) = input_block_r.range(103, 96);
			input_block.range(23, 16) = input_block_r.range(111, 104);
			input_block.range(15, 8) = input_block_r.range(119, 112);
			input_block.range(7, 0) = input_block_r.range(127, 120);

			if ((15 == cfb_byte_cnt) && (0 == cfb_bit_cnt)) { // the last bit of the last byte
				cfb_byte_cnt = 0;
				cfb_bit_cnt = 7;
			} else if (0 < cfb_bit_cnt) { // in the middle of each byte
				--cfb_bit_cnt;
			} else if (0 == cfb_bit_cnt) { // the last bit of each byte
				cfb_bit_cnt = 7;
				++cfb_byte_cnt;
			}
		}

		// CIPH_k
		cipher.process(input_block, key_r, output_block);

		// feedback for the next iteration and get the plaintext for current interation
		feedback_r[120] = ciphertext_r[cfb_byte_cnt * 8 + cfb_bit_cnt];
		plaintext_r[cfb_byte_cnt * 8 + cfb_bit_cnt] = ciphertext_r[cfb_byte_cnt * 8 + cfb_bit_cnt] ^ output_block[7];

		// write out plaintext
		next_ciphertext = false;
		if ((15 == cfb_byte_cnt) && (0 == cfb_bit_cnt)) {
		   plainTextBuffer[i]=plaintext_r;
			next_ciphertext = true;
		}

		if (next_ciphertext) {
			i++;
		}
	}
}
}