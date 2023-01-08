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
void testLz4Simple(int argc, char** argv){
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

void testLz4Compress(){
    freopen("output/lz4_output_compress.txt", "w", stdout);

    const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
	ofile.open("sample/dt_1G.txt.lz4", std::ios::binary);
	// ifile.open("sample/sample.txt", std::ios::binary);
	// ofile.open("sample/sample.txt.lz4", std::ios::binary);

	ifile.seekg(0, std::ios_base::end);
	uint64_t fileSize=ifile.tellg();
	ifile.seekg(0, std::ios_base::beg);

	uint32_t idx=dataCompression::writeLz4Header((uint8_t*)bufout.data(), fileSize);
	ofile.write(bufout.data(), idx);
	std::cout<<"write a "<<idx<<" Bytes header"<<std::endl;

	std::thread input([&]{
		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			bool last=fileSize-i-curSize==0;
			ifile.read(buf.data(), curSize);
			dataCompression::lz4CompressionInput((uint8_t*)buf.data(), curSize, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=dataCompression::lz4CompressionOutput((uint8_t*)bufout.data(), bufSize, last);
			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
			// hexdump(bufout.data(), outputSize);
			ofile.write(bufout.data(), outputSize);
		}while(!last);
	});

	input.join();
	output.join();

    idx=dataCompression::writeLz4Footer((uint8_t*)bufout.data());
	ofile.write(bufout.data(), idx);
	std::cout<<"write a "<<idx<<" Bytes footer"<<std::endl;

	std::cout<<"lz4 compress successfully"<<std::endl;

    ifile.close();
    ofile.close();
}

inline void testLz4Decompress(){
    freopen("output/lz4_output_decompress.txt", "w", stdout);

	const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	ifile.open("sample/dt_1G.txt.lz4", std::ios::binary);
	ofile.open("sample/dt_1G.txt.lz4.ori", std::ios::binary);
	// ifile.open("sample/sample.txt.lz4", std::ios::binary);
	// ofile.open("sample/sample.txt.lz4.ori", std::ios::binary);

	ifile.seekg(0, std::ios_base::end);
	uint64_t fileSize=ifile.tellg();
	ifile.seekg(0, std::ios_base::beg);

	std::thread input([&]{
		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			bool last=fileSize-i-curSize==0;
			ifile.read(buf.data(), curSize);

            if(i==0){
                hexdump(buf.data(), 20*1024);
            }

			dataCompression::lz4DecompressionInput((uint8_t*)buf.data(), curSize, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=dataCompression::lz4DecompressionOutput((uint8_t*)bufout.data(), bufSize, last);
			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
			ofile.write(bufout.data(), outputSize);
		}while(!last);
	});

	input.join();
	output.join();

	std::cout<<"snappy decompress successfully"<<std::endl;

    ifile.close();
    ofile.close();
}
} //testSnappy
