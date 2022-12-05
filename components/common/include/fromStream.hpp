#include <ap_int.h>
#include "hls_stream.h"

template <int DATA_WIDTH>
void s2mm(ap_uint<DATA_WIDTH>* out, hls::stream<ap_uint<DATA_WIDTH>> &dataStream, hls::stream<bool> &endStream) {
    int cnt=0;
    bool e;
    endStream>>e;
    while(!e){
#pragma HLS PIPELINE II = 1
        dataStream>>out[cnt++];
        endStream>>e;
    }
}

template <int DATA_WIDTH>
void s2mm(ap_uint<DATA_WIDTH>* out, int outputSize, hls::stream<ap_uint<DATA_WIDTH>> &dataStream) {
    for(int i=0;i<outputSize;i++){
#pragma HLS PIPELINE II = 1
        dataStream>>out[i];
    }
}

template <int DATA_WIDTH, int BURST_SIZE>
void s2mm(ap_uint<DATA_WIDTH>* out, hls::stream<ap_uint<DATA_WIDTH>> &dataStream, hls::stream<bool> &endStream) {
    ap_uint<DATA_WIDTH> buffer[BURST_SIZE];
    int i=0, j=0;
    bool e;
    endStream>>e;

    while(!e){
        dataStream>>buffer[j++];
        if(j==BURST_SIZE){
            for(int k=0;k<BURST_SIZE;k++) out[i+k]=buffer[k];
            i+=BURST_SIZE;
            j=0;
        }
        endStream>>e;
    }

    for(int k=0;k<j;k++) out[i+k]=buffer[k];
}

template <int DATA_WIDTH, int BURST_SIZE>
void s2mm(ap_uint<DATA_WIDTH>* out, int outputSize, hls::stream<ap_uint<DATA_WIDTH>> &dataStream) {
    ap_uint<DATA_WIDTH> buffer[BURST_SIZE];
    int times=outputSize/BURST_SIZE;
    int i=0;

    for(;i<times*BURST_SIZE;i+=BURST_SIZE){
        for(int j=0;j<BURST_SIZE;j++) dataStream>>buffer[j];
        for(int j=0;j<BURST_SIZE;j++) out[i+j]=buffer[j];        
    }

    for(int j=0;j<outputSize-i;j++) dataStream>>buffer[j];
    for(int j=0;j<outputSize-i;j++) out[i+j]=buffer[j];
}

template <int DATA_WIDTH, int BURST_SIZE>
void s2mm(ap_uint<DATA_WIDTH>* out, hls::stream<ap_uint<64>> &outputSizeStream, hls::stream<ap_uint<DATA_WIDTH>> &dataStream) {
    long long outputSize=outputSizeStream.read();
    ap_uint<DATA_WIDTH> buffer[BURST_SIZE];
    long long times=outputSize/BURST_SIZE;
    long long i=0;

    for(;i<times*BURST_SIZE;i+=BURST_SIZE){
        for(long long j=0;j<BURST_SIZE;j++) dataStream>>buffer[j];
        for(long long j=0;j<BURST_SIZE;j++) out[i+j]=buffer[j];        
    }

    for(long long j=0;j<outputSize-i;j++) dataStream>>buffer[j];
    for(long long j=0;j<outputSize-i;j++) out[i+j]=buffer[j];
}