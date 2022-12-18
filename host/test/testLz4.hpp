#pragma once

#ifdef XILINX
#include "lz4App.hpp"
#include "lz4OCLHost.hpp"
#include <memory>
#else
#include "Lz4.hpp"
#endif

#include "Helper.hpp"

namespace testLz4{
void testLz4(int argc, char** argv){
#ifdef XILINX
    // bool enable_profile = false;
    // bool lz4_stream = false;
    // compressBase::State flow = compressBase::BOTH;
    // compressBase::Level lflow = compressBase::SEQ;
    // // Driver class object
    // lz4App d(argc, argv, lflow);

    // // Design class object creating and constructor invocation
    // std::unique_ptr<lz4OCLHost> lz4(new lz4OCLHost(flow, d.getXclbin(), d.getDeviceId(), d.getBlockSize(), lz4_stream));

    // // Run API to launch the compress or decompress engine
    // d.run(lz4.get(), enable_profile);

    bool enable_profile = false;
    bool lz4_stream = true;
    compressBase::State flow = compressBase::BOTH;
    compressBase::Level lflow = compressBase::SEQ;
    // Driver class object
    lz4App d(argc, argv, lflow);

    // Design class object creating and constructor invocation
    std::unique_ptr<lz4OCLHost> lz4(new lz4OCLHost(flow, d.getXclbin(), d.getDeviceId(), d.getBlockSize(), lz4_stream));

    // Run API to launch the compress or decompress engine
    d.run(lz4.get(), enable_profile);
#else
    std::vector<uint8_t> in, out;
	readFile("sample.txt", in, out);
	std::vector<uint8_t> out2;
    out2.resize(20 * in.size());

    uint64_t inputSize=in.size(), outputSize=0, outputSize2=0;
    bool last;

    freopen("output_lz4", "w", stdout);

    std::cout<<"Start test Lz4"<<std::endl;
    std::cout<<"Original: "<<std::endl;
    hexdump(in.data(), inputSize);
    // outputSize=dataCompression::lz4Compress(in.data(), out.data(), inputSize);
    outputSize+=dataCompression::writeLz4Header(out.data(), inputSize);
    dataCompression::lz4CompressionInput(in.data(), inputSize, true);
    outputSize+=dataCompression::lz4CompressionOutput(out.data()+outputSize, out.size(), last);
    outputSize+=dataCompression::writeLz4Footer(out.data()+outputSize);
    std::cout<<"Compressed: ["<<outputSize<<" Bytes]"<<std::endl;
    hexdump(out.data(), outputSize);
	std::cout<<"------------------------"<<std::endl;
    dataCompression::lz4DecompressionInput(out.data(), outputSize, true);
    outputSize2=dataCompression::lz4DecompressionOutput(out2.data(), out2.size(), last);
    // outputSize2=dataCompression::lz4Decompress(out.data(), out2.data(), outputSize, out2.size());
    std::cout<<"Decompressed: "<<std::endl;
    hexdump(out2.data(), outputSize2);
    std::cout<<"End test Lz4"<<std::endl;
#endif
}
} //testSnappy
