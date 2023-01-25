#include "aes192Cfb8Enc.hpp"

extern "C"{
    void aes192Cfb8Enc(
        ap_uint<128> *plainTextBuffer, 
        ap_uint<192> *cipherKey, 
        ap_uint<128> *initVec, 
        ap_uint<128> *cipherTextBuffer, 
        int size){

        hls::stream<ap_uint<128>> inStream;
        hls::stream<bool> inEndStream;
        hls::stream<ap_uint<128>> outStream;
        hls::stream<bool> outEndStream;
        hls::stream<ap_uint<192>> cipherKeyStream;
        hls::stream<ap_uint<128>> initVecStream;

#pragma HLS STREAM variable = inStream depth = 16
#pragma HLS STREAM variable = inEndStream depth = 16
#pragma HLS STREAM variable = outStream depth = 16
#pragma HLS STREAM variable = outEndStream depth = 16
#pragma HLS STREAM variable = cipherKeyStream depth = 24
#pragma HLS STREAM variable = initVecStream depth = 16

        scale2s<192>(*cipherKey, cipherKeyStream);
        scale2s<128>(*initVec, initVecStream);

        mm2s<128, 256>(plainTextBuffer, size, inStream, inEndStream);

        xf::security::aes192Cfb8Encrypt(inStream, inEndStream, cipherKeyStream, initVecStream, outStream, outEndStream);

        s2mm<128, 256>(cipherTextBuffer, outStream, outEndStream);
    }
}