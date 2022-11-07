#include "desCbcDec.hpp"

extern "C"{
    void desCbcDec(ap_uint<64> *cipherTextBuffer, ap_uint<64> cipherKey, ap_uint<64> initVec, ap_uint<64> *plainTextBuffer, int size){
#pragma HLS dataflow
        hls::stream<ap_uint<64>> inStream;
        hls::stream<bool> inEndStream;
        hls::stream<ap_uint<64>> outStream;
        hls::stream<bool> outEndStream;
        hls::stream<ap_uint<64>> cipherKeyStream;
        hls::stream<ap_uint<64>> initVecStream;

#pragma HLS STREAM variable = inStream depth = 8
#pragma HLS STREAM variable = inEndStream depth = 8
#pragma HLS STREAM variable = outStream depth = 8
#pragma HLS STREAM variable = outEndStream depth = 8
#pragma HLS STREAM variable = cipherKeyStream depth = 8
#pragma HLS STREAM variable = initVecStream depth = 8

        scale2s<64>(cipherKey, cipherKeyStream);
        scale2s<64>(initVec, initVecStream);

        mm2s<64, 3>(cipherTextBuffer, size, inStream, inEndStream);

        xf::security::desCbcDecrypt(inStream, inEndStream, cipherKeyStream, initVecStream, outStream, outEndStream);

        s2mm<64, 3>(plainTextBuffer, outStream, outEndStream);
    }
}