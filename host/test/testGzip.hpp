#pragma once

#include <ctype.h>
#include <stdio.h>
#include "Gzip.hpp"

namespace testGzip{

// Input file preliminary checks
inline void inputFilePreCheck(std::ifstream& file, size_t &fileSize) {
    if (!file) {
        std::cerr << "Unable to open input file" << std::endl;
        exit(1);
    }

    file.seekg(0, file.end);
    size_t file_size = file.tellg();
    if (file_size == 0) {
        std::cerr << "File is empty!" << std::endl;
        exit(1);
    }
    file.seekg(0, file.beg);
    fileSize=file_size;
}

inline void readFile(const std::string &fileName, std::vector<uint8_t> &in, std::vector<uint8_t> &out){
	std::ifstream inFile(fileName, std::ifstream::binary);
	size_t fileSize;
	inputFilePreCheck(inFile, fileSize);
    in.resize(fileSize);
    out.resize(20 * fileSize);

	inFile.read((char*)in.data(), fileSize);
    inFile.close();
}

inline void test(){
    // uint8_t in[256], out[1024], out2[2048];
    // for(int i=0;i<256;i++) in[i]=i;
	std::vector<uint8_t> in, out;
	readFile("sample.txt", in, out);
	std::vector<uint8_t> out2;
    out2.resize(20 * in.size());

    uint64_t inputSize=in.size(), outputSize, outputSize2;

    std::cout<<"Start test Gzip"<<std::endl;
    outputSize=dataCompression::gzipCompression(in.data(), out.data(), inputSize, "sample.txt");
    hexdump(out.data(), outputSize);
	std::cout<<"------------------------"<<std::endl;
	outputSize2=dataCompression::gzipDecompression(out.data(), out2.data(), outputSize, 2048);
	hexdump(out2.data(), outputSize2);
    std::cout<<"End test Gzip"<<std::endl;
}
}