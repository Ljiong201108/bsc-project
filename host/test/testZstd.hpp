#pragma once

#ifdef XILINX
#include "zstdApp.hpp"
#include "zstdOCLHost.hpp"
#include <memory>
#else
#include "Zstd.hpp"
#endif

#include "Helper.hpp"

namespace testLz4{
void testLz4(int argc, char** argv){
#ifdef XILINX
    bool enable_profile = false;
    compressBase::State flow = compressBase::BOTH;
    compressBase::Level lflow = compressBase::SEQ;
    // Driver class object
    zstdApp d(argc, argv, lflow);

    // Design class object creating and constructor invocation
    std::unique_ptr<zstdOCLHost> zstd(new zstdOCLHost(flow, d.getXclbin(), d.getDeviceId()));

    // Run API to launch the compress or decompress engine
    d.run(zstd.get(), enable_profile);
#else
    std::vector<uint8_t> in, out;
	readFile("sample.txt", in, out);
	std::vector<uint8_t> out2;
    out2.resize(20 * in.size());

    uint64_t inputSize=in.size(), outputSize, outputSize2;

    std::cout<<"Start test Lz4"<<std::endl;
    std::cout<<"Original: "<<std::endl;
    hexdump(in.data(), inputSize);
    outputSize=dataCompression::internal::zstdCompressStream(in.data(), out.data(), inputSize);
    std::cout<<"------------------------"<<std::endl;
    std::cout<<"Compressed: "<<std::endl;
    hexdump(out.data(), outputSize);
    std::cout<<"------------------------"<<std::endl;
    std::cout<<"Stream solution: "<<std::endl;
	outputSize2=dataCompression::internal::zstdDecompressStream(out.data(), out2.data(), outputSize, out2.size());
    hexdump(out2.data(), outputSize2);
    std::cout<<"End test Snappy"<<std::endl;
#endif
}
} //testSnappy
