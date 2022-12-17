#pragma once

#include <ctype.h>
#include <stdio.h>
#include "GzipZlib.hpp"
#include "ThreadSafeFIFO.hpp"

// #include "gzipApp.hpp"
// #include "gzipOCLHost.hpp"
// #include <memory>

#include "Helper.hpp"

namespace testGzip{

inline void testGzip(){
	std::vector<uint8_t> in, out;
	readFile("sample.txt", in, out);
	std::vector<uint8_t> out2(20*out.size());

    uint64_t inputSize=in.size(), outputSize=0, outputSize2=0;
	bool last=0;

	freopen("output_gz", "w", stdout);

    std::cout<<"Start test Gzip"<<std::endl;
	std::cout<<"inputSize: "<<inputSize<<std::endl;
	std::cout<<"in.size(): "<<in.size()<<std::endl;
	std::cout<<"out.size(): "<<out.size()<<std::endl;
    // outputSize=dataCompression::gzipZlibCompression(in.data(), out.data(), in.size(), "", false);
	dataCompression::gzipZlibCompressionInput(in.data(), 1000, false);
	dataCompression::gzipZlibCompressionInput(in.data()+1000, 500, false);
	dataCompression::gzipZlibCompressionInput(in.data()+1500, inputSize-1500, true);
	uint32_t index=dataCompression::writeGzipHeader(out.data());
	while(!last){
		outputSize=dataCompression::gzipZlibCompressionOutput(out.data()+index, 500, last);
		index+=outputSize;
	}
	index=dataCompression::writeGzipFooter(out.data()+index, index, in.size());
	std::cout<<"last = "<<last<<std::endl;
	std::cout<<"index = "<<index<<std::endl;
	std::cout<<"------------------------"<<std::endl;
	// outputSize2=dataCompression::gzipZlibDecompression(out.data(), out2.data(), index);
	dataCompression::gzipZlibDecompressionInput(out.data(), 500, false);
	dataCompression::gzipZlibDecompressionInput(out.data()+500, 300, false);
	dataCompression::gzipZlibDecompressionInput(out.data()+800, index-800, true);
	last=0;
	while(!last) outputSize2+=dataCompression::gzipZlibDecompressionOutput(out2.data()+outputSize2, 800, last);
	std::cout<<"last = "<<last<<std::endl;
	hexdump(out2.data(), outputSize2, "output_gz");
    std::cout<<"End test Gzip"<<std::endl;
}

// inline void testZlib(){
// 	std::vector<uint8_t> in, out;
// 	readFile("sample.txt", in, out);
// 	std::vector<uint8_t> out2(20*out.size());

//     uint64_t inputSize=in.size(), outputSize, outputSize2;

//     std::cout<<"Start test Zlib"<<std::endl;
//     outputSize=dataCompression::gzipZlibCompression(in.data(), out.data(), in.size(), "", true);
// 	std::cout<<"------------------------"<<std::endl;
// 	outputSize2=dataCompression::gzipZlibDecompression(out.data(), out2.data(), outputSize);
//     std::cout<<"End test Zlib"<<std::endl;
// }

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