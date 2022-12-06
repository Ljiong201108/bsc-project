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
    // std::vector<uint8_t> in, out;
	// readFile("sample.txt", in, out);
	// std::vector<uint8_t> out2;
    // out2.resize(20 * in.size());

    // uint64_t inputSize=in.size(), outputSize, outputSize2;

    // std::cout<<"Start test Snappy"<<std::endl;
    // outputSize=dataCompression::internal::snappyCompressStream(in.data(), out.data(), inputSize);
    // hexdump(out.data(), outputSize);
	// std::cout<<"------------------------"<<std::endl;
    // std::cout<<"MM solution: "<<std::endl;
    // dataCompression::internal::snappyDecompressMM(out.data(), out2.data(), outputSize, out2.size());
    // std::cout<<"Stream solution: "<<std::endl;
	// dataCompression::internal::snappyDecompressStream(out.data(), out2.data(), outputSize, out2.size());
    // std::cout<<"End test Snappy"<<std::endl;
#endif
}
} //testSnappy
