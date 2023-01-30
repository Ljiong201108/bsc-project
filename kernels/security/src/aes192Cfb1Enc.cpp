#include "aes192Cfb1Enc.hpp"

extern "C"{
void aes192Cfb1Enc(
	ap_uint<128> *plainTextBuffer, 
	ap_uint<192> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *cipherTextBuffer, 
	int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer 

	ap_uint<192> key_r=cipherKey[0];
	ap_uint<128> iv=initVec[0];

	xf::security::aesEnc<192> cipher;
	cipher.updateKey(key_r);

	// intermediate registers to perform the encryption chain
	bool next_plaintext = true;
	ap_uint<128> plaintext_r = 0;
	ap_uint<128> feedback_r = 0;
	ap_uint<128> input_block = 0;
	ap_uint<128> output_block = 0;
	ap_uint<128> ciphertext_r = 0;
	ap_uint<4> cfb_byte_cnt = 0;
	ap_uint<3> cfb_bit_cnt = 7;

	// set the initialization for ture
	bool initialization = true;

encryption_cfb1_loop:
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
		} else { // after first iteration, input_blcok is comprised by 127 bits of IV and 1 bit of ciphertext
			ap_uint<128> ibt;
			ibt.range(127, 121) = input_block(126, 120);
			ibt[120] = feedback_r[120];
			ibt.range(119, 113) = input_block(118, 112);
			ibt[112] = input_block[127];
			ibt.range(111, 105) = input_block(110, 104);
			ibt[104] = input_block[119];
			ibt.range(103, 97) = input_block(102, 96);
			ibt[96] = input_block[111];
			ibt.range(95, 89) = input_block(94, 88);
			ibt[88] = input_block[103];
			ibt.range(87, 81) = input_block(86, 80);
			ibt[80] = input_block[95];
			ibt.range(79, 73) = input_block(78, 72);
			ibt[72] = input_block[87];
			ibt.range(71, 65) = input_block(70, 64);
			ibt[64] = input_block[79];
			ibt.range(63, 57) = input_block(62, 56);
			ibt[56] = input_block[71];
			ibt.range(55, 49) = input_block(54, 48);
			ibt[48] = input_block[63];
			ibt.range(47, 41) = input_block(46, 40);
			ibt[40] = input_block[55];
			ibt.range(39, 33) = input_block(38, 32);
			ibt[32] = input_block[47];
			ibt.range(31, 25) = input_block(30, 24);
			ibt[24] = input_block[39];
			ibt.range(23, 17) = input_block(22, 16);
			ibt[16] = input_block[31];
			ibt.range(15, 9) = input_block(14, 8);
			ibt[8] = input_block[23];
			ibt.range(7, 1) = input_block(6, 0);
			ibt[0] = input_block[15];
			input_block = ibt;

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

		// feedback for the next iteration and get the ciphertext for current interation
		ciphertext_r[cfb_byte_cnt * 8 + cfb_bit_cnt] = plaintext_r[cfb_byte_cnt * 8 + cfb_bit_cnt] ^ output_block[7];
		feedback_r[120] = ciphertext_r[cfb_byte_cnt * 8 + cfb_bit_cnt];

		// write out ciphertext and decide whether to read a new plaintext block or not
		next_plaintext = false;
		if ((15 == cfb_byte_cnt) && (0 == cfb_bit_cnt)) {
			cipherTextBuffer[i]=ciphertext_r;
			next_plaintext = true;
		}

		if (next_plaintext) {
			i++;
		}
	}
}
}