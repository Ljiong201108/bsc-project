#pragma once

#include <ctype.h>
#include <stdio.h>
#include "GzipZlib.hpp"

// #include "gzipApp.hpp"
// #include "gzipOCLHost.hpp"
// #include <memory>

#include "Helper.hpp"

namespace testGzip{

inline void testGzip(){
    // uint8_t in[256], out[1024], out2[2048];
    // for(int i=0;i<256;i++) in[i]=i;
	std::vector<uint8_t> in, out;
	readFile("sample.txt", in, out);
	std::vector<uint8_t> out2;
    out2.resize(20 * in.size());

    uint64_t inputSize=in.size(), outputSize, outputSize2;

    std::cout<<"Start test Gzip"<<std::endl;
    outputSize=dataCompression::gzipZlibCompression(in.data(), out.data(), inputSize, "", false);
    hexdump(out.data(), outputSize);
	std::cout<<"------------------------"<<std::endl;
	outputSize2=dataCompression::gzipZlibDecompression(out.data(), out2.data(), outputSize, 2048, true);
	hexdump(out2.data(), outputSize2);
    std::cout<<"End test Gzip"<<std::endl;
}

inline void testZlib(){
    // uint8_t in[256], out[1024], out2[2048];
    // for(int i=0;i<256;i++) in[i]=i;
	std::vector<uint8_t> in, out;
	readFile("sample.txt", in, out);
	std::vector<uint8_t> out2;
    out2.resize(20 * in.size());

    uint64_t inputSize=in.size(), outputSize, outputSize2;

    std::cout<<"Start test Gzip"<<std::endl;
    outputSize=dataCompression::gzipZlibCompression(in.data(), out.data(), inputSize, "", true);
    hexdump(out.data(), outputSize);
	std::cout<<"------------------------"<<std::endl;
	outputSize2=dataCompression::gzipZlibDecompression(out.data(), out2.data(), outputSize, 2048, true);
	hexdump(out2.data(), outputSize2);
    std::cout<<"End test Gzip"<<std::endl;
}

// inline void testXilinxGzipZlib(int argc, char** argv){
//     bool enable_profile = true;
//     compressBase::State flow = compressBase::BOTH;
//     gzipBase::d_type decKernelType = gzipBase::FULL;
//     bool swpipline_option = true;

//     // Driver class object
//     gzipApp d(argc, argv, swpipline_option);

//     // Design class object creating and constructor invocation
//     std::unique_ptr<gzipOCLHost> gzip(
//         new gzipOCLHost(flow, d.getXclbin(), d.getDeviceId(), decKernelType, d.getDesignFlow()));

//     // Run API to launch the compress or decompress engine
//     d.run(gzip.get(), enable_profile);
// }
}