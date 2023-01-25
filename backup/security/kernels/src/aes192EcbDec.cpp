#include "aes192EcbDec.hpp"

extern "C"{
    void aes192EcbDec(
        ap_uint<128> *cipherTextBuffer, 
        ap_uint<192> *cipherKey, 
        ap_uint<128> *plainTextBuffer, 
        int size){

        hls::stream<ap_uint<128>> inStream;
        hls::stream<bool> inEndStream;
        hls::stream<ap_uint<128>> outStream;
        hls::stream<bool> outEndStream;
        hls::stream<ap_uint<192>> cipherKeyStream;

#pragma HLS STREAM variable = inStream depth = 16
#pragma HLS STREAM variable = inEndStream depth = 16
#pragma HLS STREAM variable = outStream depth = 16
#pragma HLS STREAM variable = outEndStream depth = 16
#pragma HLS STREAM variable = cipherKeyStream depth = 24

        scale2s<192>(*cipherKey, cipherKeyStream);

        mm2s<128, 256>(cipherTextBuffer, size, inStream, inEndStream);

        xf::security::aes192EcbDecrypt(inStream, inEndStream, cipherKeyStream, outStream, outEndStream);

        s2mm<128, 256>(plainTextBuffer, outStream, outEndStream);
    }
}