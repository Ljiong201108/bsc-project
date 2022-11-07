#include <ap_int.h>
#include "hls_stream.h"

template <int DATA_WIDTH>
void mm2s(ap_uint<DATA_WIDTH>* in, int inputSize, hls::stream<ap_uint<DATA_WIDTH>> &dataStream, hls::stream<bool> &endStream) {
    for(int i=0;i<inputSize;i++){
#pragma HLS PIPELINE II = 1
        dataStream<<in[i];
        endStream<<0;
    }
    endStream<<1;
}

template <int DATA_WIDTH>
void scale2s(ap_uint<DATA_WIDTH> number, hls::stream<ap_uint<DATA_WIDTH>> &stream){
    stream<<number;
}

template <int DATA_WIDTH, int BURST_SIZE>
void mm2s(ap_uint<DATA_WIDTH>* in, int inputSize, hls::stream<ap_uint<DATA_WIDTH>> &dataStream, hls::stream<bool> &endStream) {
    ap_uint<DATA_WIDTH> buffer[BURST_SIZE];
    int times=inputSize/BURST_SIZE;
    int i=0;

    for(;i<times*BURST_SIZE;i+=BURST_SIZE){
        for(int j=0;j<BURST_SIZE;j++) buffer[j]=in[i+j];        
        for(int j=0;j<BURST_SIZE;j++) dataStream<<buffer[j], endStream<<0;
    }

    for(int j=0;j<inputSize-i;j++) buffer[j]=in[i+j];
    for(int j=0;j<inputSize-i;j++) dataStream<<buffer[j], endStream<<0;

    endStream<<1;
}