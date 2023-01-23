#include "aes128CbcEnc.hpp"

extern "C"{
    void aes128CbcEnc(
        ap_uint<128> *plainTextBuffer, 
        ap_uint<128> *cipherKey, 
        ap_uint<128> *initVec, 
        ap_uint<128> *cipherTextBuffer, 
        int size){

        hls::stream<ap_uint<128>> inStream;
        hls::stream<bool> inEndStream;
        hls::stream<ap_uint<128>> outStream;
        hls::stream<bool> outEndStream;
        hls::stream<ap_uint<128>> cipherKeyStream;
        hls::stream<ap_uint<128>> initVecStream;

#pragma HLS STREAM variable = inStream depth = 1024
#pragma HLS STREAM variable = inEndStream depth = 1024
#pragma HLS STREAM variable = outStream depth = 16
#pragma HLS STREAM variable = outEndStream depth = 16
#pragma HLS STREAM variable = cipherKeyStream depth = 16
#pragma HLS STREAM variable = initVecStream depth = 16

aes128CbcEnc:
#pragma HLS dataflow

        scale2s<128>(*cipherKey, cipherKeyStream);
        scale2s<128>(*initVec, initVecStream);

        mm2s<128, 256>(plainTextBuffer, size, inStream, inEndStream);

        xf::security::aes128CbcEncrypt(inStream, inEndStream, cipherKeyStream, initVecStream, outStream, outEndStream);

        s2mm<128, 256>(cipherTextBuffer, outStream, outEndStream);
    }
}