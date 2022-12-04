#include "aes128CbcDec.hpp"

extern "C"{
    void aes128CbcDec(
        ap_uint<128> *cipherTextBuffer, 
        ap_uint<128> *cipherKey, 
        ap_uint<128> *initVec, 
        ap_uint<128> *plainTextBuffer, 
        int size){

        hls::stream<ap_uint<128>> inStream;
        hls::stream<bool> inEndStream;
        hls::stream<ap_uint<128>> outStream;
        hls::stream<bool> outEndStream;
        hls::stream<ap_uint<128>> cipherKeyStream;
        hls::stream<ap_uint<128>> initVecStream;

#pragma HLS STREAM variable = inStream depth = 16
#pragma HLS STREAM variable = inEndStream depth = 16
#pragma HLS STREAM variable = outStream depth = 16
#pragma HLS STREAM variable = outEndStream depth = 16
#pragma HLS STREAM variable = cipherKeyStream depth = 16
#pragma HLS STREAM variable = initVecStream depth = 16

        scale2s<128>(*cipherKey, cipherKeyStream);
        scale2s<128>(*initVec, initVecStream);

        mm2s<128, 256>(cipherTextBuffer, size, inStream, inEndStream);

        xf::security::aes128CbcDecrypt(inStream, inEndStream, cipherKeyStream, initVecStream, outStream, outEndStream);

        s2mm<128, 256>(plainTextBuffer, outStream, outEndStream);
    }
}