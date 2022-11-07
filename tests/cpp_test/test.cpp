#include<bits/stdc++.h>
using namespace std;
typedef long long ll;

template <int BURST_SIZE>
void s2mm(int* out, ifstream &dataStream, ifstream &endStream) {
    int buffer[BURST_SIZE];
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

template <int BURST_SIZE>
void mm2s(int* in, int inputSize, ofstream &dataStream, ofstream &endStream) {
    int buffer[BURST_SIZE];
    int times=inputSize/BURST_SIZE;
    int i=0;

    for(;i<times*BURST_SIZE;i+=BURST_SIZE){
        for(int j=0;j<BURST_SIZE;j++) buffer[j]=in[i+j];        
        for(int j=0;j<BURST_SIZE;j++) dataStream<<buffer[j]<<endl, endStream<<0<<endl;
    }

    for(int j=0;j<inputSize-i;j++) buffer[j]=in[i+j];
    for(int j=0;j<inputSize-i;j++) dataStream<<buffer[j]<<endl, endStream<<0<<endl;

    endStream<<1;
}

void test_s2mm(){
    int out[4];
    ifstream data, end;
    data.open("data.txt");
    end.open("end.txt");

    s2mm<3>(out, data, end);

    for(int i=0;i<4;i++) cout<<hex<<out[i]<<" ";
    cout<<endl;

    data.close();
    end.close();
}

void test_mm2s(){
    int in[4]={0x1234, 0x2345, 0x3456, 0x4567};
    ofstream data, end;
    data.open("data.txt");
    end.open("end.txt");

    mm2s<3>(in, 4, data, end);

    data.close();
    end.close();
}

void Main(){
    test_mm2s();
    test_s2mm();
}

int main(){
    #ifdef DEBUG
    freopen("in.txt", "r", stdin);
    //freopen("out.txt", "w", stdout);
    #endif

    Main();
    
    return 0;
}