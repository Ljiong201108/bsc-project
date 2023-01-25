#include "aes128Cfb8Enc.hpp"

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
void aesCfb8Encrypt(
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

	// set the initialization for ture
	bool initialization = true;

	bool e = plaintext_e.read();

encryption_cfb8_loop:
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
			ciphertext.write(ciphertext_r);
			ciphertext_e.write(0);
			next_plaintext = true;
		}

		if (next_plaintext) {
			e = plaintext_e.read();
		}
	}

	ciphertext_e.write(1);

} // end aesCfb8Encrypt

extern "C"{
void aes128Cfb8Enc(
	ap_uint<128> *plainTextBuffer, 
	ap_uint<128> *cipherKey, 
	ap_uint<128> *initVec, 
	ap_uint<128> *cipherTextBuffer, 
	int size){

#pragma HLS interface m_axi offset = slave bundle = gmem0 port = plainTextBuffer 
#pragma HLS interface m_axi offset = slave bundle = gmem1 port = cipherTextBuffer 

	ap_uint<128> key=cipherKey[0];
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

	aesCfb8Encrypt<128>(inStream, inEndStream, key, iv, outStream, outEndStream);

	s2mm(cipherTextBuffer, outStream, outEndStream);
}
}