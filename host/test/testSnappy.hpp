#pragma once

#ifdef XILINX
#include "snappyApp.hpp"
#include "snappyOCLHost.hpp"
#include <memory>
#else
#include "Snappy.hpp"
#endif

#include "Helper.hpp"

namespace testSnappy{
void testSnappy(int argc, char** argv){
#ifdef XILINX
    bool enable_profile = false;
    compressBase::State flow = compressBase::BOTH;
    compressBase::Level lflow = compressBase::SEQ;
    // Driver class object
    snappyApp d(argc, argv, lflow);

    // Design class object creating and constructor invocation
    std::unique_ptr<snappyOCLHost> snappy(new snappyOCLHost(flow, d.getXclbin(), d.getDeviceId(), d.getBlockSize()));

    // Run API to launch the compress or decompress engine
    d.run(snappy.get(), enable_profile);
#else
    std::vector<uint8_t> in, out;
	readFile("sample.txt", in, out);
	std::vector<uint8_t> out2;
    out2.resize(20 * in.size());

    uint64_t inputSize=in.size(), outputSize, outputSize2;

    std::cout<<"Start test Snappy"<<std::endl;
    outputSize=dataCompression::internal::snappyCompressStream(in.data(), out.data(), inputSize);
    hexdump(out.data(), outputSize);
	std::cout<<"------------------------"<<std::endl;
	outputSize2=dataCompression::internal::snappyDecompressStream(out.data(), out2.data(), outputSize);
	hexdump(out2.data(), outputSize2);
    std::cout<<"End test Snappy"<<std::endl;
#endif
}
} //testSnappy
