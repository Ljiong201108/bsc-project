#include "aes256OfbDec.hpp"

extern "C"{
    void aes256OfbDec(
        ap_uint<128> *cipherTextBuffer, 
        ap_uint<256> *cipherKey, 
        ap_uint<128> *initVec, 
        ap_uint<128> *plainTextBuffer, 
        int size){

        hls::stream<ap_uint<128>> inStream;
        hls::stream<bool> inEndStream;
        hls::stream<ap_uint<128>> outStream;
        hls::stream<bool> outEndStream;
        hls::stream<ap_uint<256>> cipherKeyStream;
        hls::stream<ap_uint<128>> initVecStream;

#pragma HLS STREAM variable = inStream depth = 16
#pragma HLS STREAM variable = inEndStream depth = 16
#pragma HLS STREAM variable = outStream depth = 16
#pragma HLS STREAM variable = outEndStream depth = 16
#pragma HLS STREAM variable = cipherKeyStream depth = 32
#pragma HLS STREAM variable = initVecStream depth = 16

        scale2s<256>(*cipherKey, cipherKeyStream);
        scale2s<128>(*initVec, initVecStream);

        mm2s<128, 256>(cipherTextBuffer, size, inStream, inEndStream);

        xf::security::aes256OfbDecrypt(inStream, inEndStream, cipherKeyStream, initVecStream, outStream, outEndStream);

        s2mm<128, 256>(plainTextBuffer, outStream, outEndStream);
    }
}