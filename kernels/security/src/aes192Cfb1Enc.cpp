#include "aes192Cfb1Enc.hpp"

void mm2s(ap_uint<128>* in, int inputSize, hls::stream<ap_uint<128>> &dataStream, hls::stream<bool> &endStream) {
	for(int i=0;i<inputSize;i++){
		dataStream<<in[i];
		endStream<<0;
	}
	endStream<<1;
}

void s2mm(ap_uint<128>* out, hls::stream<ap_uint<128>> &dataStream, hls::stream<bool> &endStream) {
	int cnt=0;
	bool e;
	endStream>>e;
	while(!e){
		dataStream>>out[cnt++];
		endStream>>e;
	}
}

template <unsigned int _keyWidth = 256>
void aesCfb1Encrypt(
	// stream in
	hls::stream<ap_uint<128> >& plaintext,
	hls::stream<bool>& plaintext_e,
	// input cipherkey and initialization vector
	ap_uint<_keyWidth> cipherkey,
	ap_uint<128> initialization_vector,
	// stream out
	hls::stream<ap_uint<128> >& ciphertext,
	hls::stream<bool>& ciphertext_e) {
	// register cipherkey
	ap_uint<_keyWidth> key_r = cipherkey;

	xf::security::aesEnc<_keyWidth> cipher;
	cipher.updateKey(key_r);

	// register IV
	ap_uint<128> IV = initialization_vector;

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

	bool e = plaintext_e.read();

encryption_cfb1_loop:
	while (!e) {
#pragma HLS PIPELINE
		// read a block of plaintext, 128 bits
		if (next_plaintext) { // mode CFB1/CFB8 needs multiple iteration to process one plaintext block
			plaintext_r = plaintext.read();
		}

		// calculate input_block
		if (initialization) { // first iteration, input_block is IV
			input_block = IV;
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
			ciphertext.write(ciphertext_r);
			ciphertext_e.write(0);
			next_plaintext = true;
		}

		if (next_plaintext) {
			e = plaintext_e.read();
		}
	}

	ciphertext_e.write(1);

} // end aesCfb1Encrypt

extern "C"{
	void aes192Cfb1Enc(
		ap_uint<128> *plainTextBuffer, 
		ap_uint<192> *cipherKey, 
		ap_uint<128> *initVec, 
		ap_uint<128> *cipherTextBuffer, 
		int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer 

	ap_uint<192> key=cipherKey[0];
	ap_uint<128> iv=initVec[0];

main:

	hls::stream<ap_uint<128>> inStream;
	hls::stream<bool> inEndStream;
	hls::stream<ap_uint<128>> outStream;
	hls::stream<bool> outEndStream;

#pragma HLS STREAM variable = inStream depth = 4096
#pragma HLS STREAM variable = inEndStream depth = 4096
#pragma HLS STREAM variable = outStream depth = 4096
#pragma HLS STREAM variable = outEndStream depth = 4096
#pragma HLS resource variable = inStream core = FIFO_LUTRAM
#pragma HLS resource variable = inEndStream core = FIFO_LUTRAM
#pragma HLS resource variable = outStream core = FIFO_LUTRAM
#pragma HLS resource variable = outEndStream core = FIFO_LUTRAM

#pragma HLS dataflow

	mm2s(plainTextBuffer, size, inStream, inEndStream);

	aesCfb1Encrypt<192>(inStream, inEndStream, key, iv, outStream, outEndStream);

	s2mm(cipherTextBuffer, outStream, outEndStream);
}
}