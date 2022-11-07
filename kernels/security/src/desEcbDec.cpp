
#include "desEcbDec.hpp"

extern "C"{
    void desEcbDec(ap_uint<64> *cipherTextBuffer, ap_uint<64> cipherKey, ap_uint<64> *plainTextBuffer, int size){
#pragma HLS dataflow
        hls::stream<ap_uint<64>> inStream;
        hls::stream<bool> inEndStream;
        hls::stream<ap_uint<64>> outStream;
        hls::stream<bool> outEndStream;
        hls::stream<ap_uint<64>> cipherKeyStream;

#pragma HLS STREAM variable = inStream depth = 8
#pragma HLS STREAM variable = inEndStream depth = 8
#pragma HLS STREAM variable = outStream depth = 8
#pragma HLS STREAM variable = outEndStream depth = 8
#pragma HLS STREAM variable = cipherKeyStream depth = 8

        scale2s<64>(cipherKey, cipherKeyStream);

        mm2s<64, 256>(cipherTextBuffer, size, inStream, inEndStream);

        xf::security::desEcbDecrypt(inStream, inEndStream, cipherKeyStream, outStream, outEndStream);

        s2mm<64, 256>(plainTextBuffer, outStream, outEndStream);
    }
}