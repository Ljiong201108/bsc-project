#include "aes256CcmDec.hpp"

static void pushStream(
    unsigned long long numMessage, 
    ap_uint<128> *inBuffer,
    ap_uint<128> *ADBuffer, 
    ap_uint<64> *numBlockInPerMessage, 
    ap_uint<64> *numBlockADPerMessage, 
    hls::stream<ap_uint<128>> &cipherStream,
    hls::stream<ap_uint<128>> &ADStream,
    hls::stream<ap_uint<64>> &numByteInStream,
    hls::stream<ap_uint<64>> &numByteADStream,
    hls::stream<bool> &endStream){

    for(unsigned int i=0;i<numMessage;i++){
        ap_uint<64> curNumBlockIn=numBlockInPerMessage[i];
        ap_uint<64> curNumBlockAD=numBlockADPerMessage[i];
        mm2s<128, 256>(inBuffer, curNumBlockIn, cipherStream);
        mm2s<128, 256>(ADBuffer, curNumBlockAD, ADStream);
        inBuffer+=curNumBlockIn;
        ADBuffer+=curNumBlockAD;
        numByteInStream<<curNumBlockIn*16;
        numByteADStream<<curNumBlockAD*16;
        endStream<<0;
    } 
    endStream<<1;
}

static void pullStream(
    ap_uint<128> *outBuffer, 
    ap_uint<128> *tagBuffer,
    hls::stream<ap_uint<128>> &outStream,
    hls::stream<ap_uint<64>> &numByteOutStream,
    hls::stream<ap_uint<128>> &tagStream,
    hls::stream<bool> &endStream
    ){
    
    bool e=endStream.read();
    while(!e){
        ap_uint<64> curNumBlockOut=numByteOutStream.read()/16;
        s2mm<128, 256>(outBuffer, curNumBlockOut, outStream);
        tagStream>>tagBuffer[0];
        outBuffer+=curNumBlockOut;
        tagBuffer++;
        endStream>>e;
    }
}

extern "C"{
void aes256CcmDec(
    ap_uint<128> *inBuffer, 
    ap_uint<256> *cipherKey, 
    ap_uint<56> *nonce,
    ap_uint<128> *ADBuffer, 
    ap_uint<64> *numBlockInPerMessage,
    ap_uint<64> *numBlockADPerMessage,  
    ap_uint<128> *outBuffer, 
    ap_uint<128> *tagBuffer,
    unsigned long long numMessage){

    hls::stream<ap_uint<128>> cipherStream;
    hls::stream<ap_uint<56>> nonceStream;
    hls::stream<ap_uint<128>> ADStream;
    hls::stream<ap_uint<64>> numByteADStream;
    hls::stream<ap_uint<64>> numByteInStream;
    hls::stream<bool> endStream;
    hls::stream<ap_uint<128>> outStream;
    hls::stream<ap_uint<64>> numByteOutStream;
    hls::stream<ap_uint<256>> cipherKeyStream;
    hls::stream<ap_uint<128>> tagStream;
    hls::stream<bool> endTagStream;

#pragma HLS STREAM variable = cipherStream depth = 4
#pragma HLS STREAM variable = nonceStream depth = 4
#pragma HLS STREAM variable = ADStream depth = 4
#pragma HLS STREAM variable = numByteADStream depth = 4
#pragma HLS STREAM variable = numByteOutStream depth = 4
#pragma HLS STREAM variable = endStream depth = 4
#pragma HLS STREAM variable = outStream depth = 4
#pragma HLS STREAM variable = numByteInStream depth = 4
#pragma HLS STREAM variable = cipherKeyStream depth = 4
#pragma HLS STREAM variable = tagStream depth = 4
#pragma HLS STREAM variable = endTagStream depth = 4

    scale2s<256>(*cipherKey, cipherKeyStream);
    scale2s<56>(*nonce, nonceStream);

    pushStream(numMessage, inBuffer, ADBuffer, numBlockInPerMessage, numBlockADPerMessage, cipherStream, ADStream, numByteInStream, numByteADStream, endStream);

    xf::security::aes256CcmDecrypt(cipherStream, cipherKeyStream, nonceStream, ADStream, numByteADStream, numByteInStream, endStream, outStream, numByteOutStream, tagStream, endTagStream);

    pullStream(outBuffer, tagBuffer, outStream, numByteOutStream, tagStream, endTagStream);
}
}