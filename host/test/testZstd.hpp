#pragma once

#ifdef XILINX
#include "zstdApp.hpp"
#include "zstdOCLHost.hpp"
#include <memory>
#else
#include "Zstd.hpp"
#endif

#include "Helper.hpp"

namespace testZstd{
void testZstd(int argc, char** argv){
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
	// readFile("sample.txt", in, out);
    readFile("sample.txt", in, out);
	std::vector<uint8_t> out2;
    out2.resize(20 * in.size());

    uint64_t inputSize=in.size(), outputSize=0, outputSize2=0;
    bool last;

    freopen("output_zstd", "w", stdout);

    std::cout<<"Start test Zstd"<<std::endl;
    std::cout<<"Original: "<<std::endl;
    std::cout<<"in.size(): "<<in.size()<<std::endl;
    std::cout<<"out.size(): "<<out.size()<<std::endl;
    // outputSize=dataCompression::zstdCompress(in.data(), out.data(), inputSize);
    dataCompression::zstdCompressionInput(in.data(), inputSize, true);
    outputSize+=dataCompression::zstdCompressionOutput(out.data(), out.size(), last);
    std::cout<<"Compressed: "<<std::endl;
    hexdump(out.data(), outputSize);
    std::cout<<"------------------------"<<std::endl;
	// outputSize2=dataCompression::zstdDecompress(out.data(), out2.data(), outputSize);
    dataCompression::zstdDecompressionInput(out.data(), outputSize, true);
    outputSize2=dataCompression::zstdDecompressionOutput(out2.data(), out2.size(), last);
    std::cout<<"last: "<<last<<std::endl;
    hexdump(out2.data(), outputSize2);
    std::cout<<"End test Zstd"<<std::endl;
#endif
}
} //testZstd
