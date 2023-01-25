#include "aes256GcmDec.hpp"

static void pushStream(
    unsigned long long numMessage, 
    ap_uint<128> *inBuffer,
    ap_uint<128> *ADBuffer, 
    ap_uint<64> *numBlockInPerMessage, 
    ap_uint<64> *numBlockADPerMessage, 
    hls::stream<ap_uint<128>> &inStream,
    hls::stream<ap_uint<128>> &ADStream,
    hls::stream<ap_uint<64>> &numBitInStream,
    hls::stream<ap_uint<64>> &numBitADStream,
    hls::stream<bool> &endStream){

    for(unsigned int i=0;i<numMessage;i++){
        ap_uint<64> curNumBlockIn=numBlockInPerMessage[i];
        ap_uint<64> curNumBlockAD=numBlockADPerMessage[i];
        mm2s<128, 256>(inBuffer, curNumBlockIn, inStream);
        mm2s<128, 256>(ADBuffer, curNumBlockAD, ADStream);
        inBuffer+=curNumBlockIn;
        ADBuffer+=curNumBlockAD;
        numBitInStream<<curNumBlockIn*128;
        numBitADStream<<curNumBlockAD*128;
        endStream<<0;
    } 
    endStream<<1;
}

static void pullStream(
    ap_uint<128> *outBuffer, 
    ap_uint<128> *tagBuffer,
    hls::stream<ap_uint<128>> &outStream,
    hls::stream<ap_uint<64>> &numBitOutStream,
    hls::stream<ap_uint<128>> &tagStream,
    hls::stream<bool> &endStream
    ){
    
    bool e=endStream.read();
    while(!e){
        ap_uint<64> curNumBlockOut=numBitOutStream.read()/128;
        s2mm<128, 256>(outBuffer, curNumBlockOut, outStream);
        tagStream>>tagBuffer[0];
        outBuffer+=curNumBlockOut;
        tagBuffer++;
        endStream>>e;
    }
}

extern "C"{
void aes256GcmDec(
    ap_uint<128> *inBuffer, 
    ap_uint<256> *cipherKey, 
    ap_uint<96> *IVBuffer,
    ap_uint<128> *ADBuffer, 
    ap_uint<64> *numBlockInPerMessage,
    ap_uint<64> *numBlockADPerMessage,  
    ap_uint<128> *outBuffer, 
    ap_uint<128> *tagBuffer,
    unsigned long long numMessage){

    hls::stream<ap_uint<128>> inStream;
    hls::stream<ap_uint<96>> IVStream;
    hls::stream<ap_uint<128>> ADStream;
    hls::stream<ap_uint<64>> numBitADStream;
    hls::stream<ap_uint<64>> numBitInStream;
    hls::stream<bool> endStream;
    hls::stream<ap_uint<128>> outStream;
    hls::stream<ap_uint<64>> numBitOutStream;
    hls::stream<ap_uint<256>> cipherKeyStream;
    hls::stream<ap_uint<128>> tagStream;
    hls::stream<bool> endTagStream;

#pragma HLS STREAM variable = inStream depth = 4
#pragma HLS STREAM variable = IVStream depth = 4
#pragma HLS STREAM variable = ADStream depth = 4
#pragma HLS STREAM variable = numBitADStream depth = 4
#pragma HLS STREAM variable = numBitOutStream depth = 4
#pragma HLS STREAM variable = endStream depth = 4
#pragma HLS STREAM variable = outStream depth = 4
#pragma HLS STREAM variable = numBitInStream depth = 4
#pragma HLS STREAM variable = cipherKeyStream depth = 4
#pragma HLS STREAM variable = tagStream depth = 4
#pragma HLS STREAM variable = endTagStream depth = 4

    scale2s<256>(*cipherKey, cipherKeyStream);
    scale2s<96>(*IVBuffer, IVStream);

    pushStream(numMessage, inBuffer, ADBuffer, numBlockInPerMessage, numBlockADPerMessage, inStream, ADStream, numBitInStream, numBitADStream, endStream);

    xf::security::aes256GcmDecrypt(inStream, cipherKeyStream, IVStream, ADStream, numBitADStream, numBitInStream, endStream, outStream, numBitOutStream, tagStream, endTagStream);

    pullStream(outBuffer, tagBuffer, outStream, numBitOutStream, tagStream, endTagStream);
}
}