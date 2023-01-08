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
void testSnappySimple(int argc, char** argv){
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
	readFile("sample/sample.txt", in, out);
	std::vector<uint8_t> out2;
    out2.resize(20 * in.size());

    freopen("output_snappy", "w", stdout);
    uint64_t inputSize=in.size(), outputSize=0, outputSize2=0;
    bool last;

    std::cout<<"Start test Snappy"<<std::endl;
    // outputSize=dataCompression::snappyCompress(in.data(), out.data(), inputSize);
    outputSize+=dataCompression::writeSnappyHeader(out.data());
    dataCompression::snappyCompressionInput(in.data(), inputSize, true);
    outputSize+=dataCompression::snappyCompressionOutput(out.data()+outputSize, out.size(), last);
    std::cout<<"Compressed: "<<outputSize<<" Bytes"<<std::endl;
    hexdump(out.data(), outputSize);
	std::cout<<"------------------------"<<std::endl;
    // outputSize2=dataCompression::snappyDecompress(out.data(), out2.data(), outputSize);
    dataCompression::snappyDecompressionInput(out.data(), outputSize, true);
    outputSize2=dataCompression::snappyDecompressionOutput(out2.data(), out2.size(), last);
    hexdump(out2.data(), outputSize2);
    std::cout<<"End test Snappy"<<std::endl;
#endif
}

void testSnappyCompress(){
	freopen("smaple/snappy_output_compress.txt", "w", stdout);

    const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	// ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
	// ofile.open("sample/dt_1G.txt.sz", std::ios::binary);
	ifile.open("sample/sample.txt", std::ios::binary);
	ofile.open("sample/sample.txt.sz", std::ios::binary);

	ifile.seekg(0, std::ios_base::end);
	uint64_t fileSize=ifile.tellg();
	ifile.seekg(0, std::ios_base::beg);

	uint32_t idx=dataCompression::writeSnappyHeader((uint8_t*)bufout.data());
	ofile.write(bufout.data(), idx);
	std::cout<<"write a "<<idx<<" Bytes header"<<std::endl;

	std::thread input([&]{
		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			bool last=fileSize-i-curSize==0;
			ifile.read(buf.data(), curSize);
			dataCompression::snappyCompressionInput((uint8_t*)buf.data(), curSize, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=dataCompression::snappyCompressionOutput((uint8_t*)bufout.data(), bufSize, last);
			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
			// hexdump(bufout.data(), outputSize);
			ofile.write(bufout.data(), outputSize);
		}while(!last);
	});

	input.join();
	output.join();

	std::cout<<"snappy compress successfully"<<std::endl;

    ifile.close();
    ofile.close();
}

inline void testSnappyDecompress(){
    freopen("output/snappy_output_decompress.txt", "w", stdout);

	const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	ifile.open("sample/dt_1G.txt.sz", std::ios::binary);
	ofile.open("sample/dt_1G.txt.sz.ori", std::ios::binary);
	// ifile.open("sample/sample.txt.sz", std::ios::binary);
	// ofile.open("sample/sample.txt.sz.ori", std::ios::binary);

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

			dataCompression::snappyDecompressionInput((uint8_t*)buf.data(), curSize, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=dataCompression::snappyDecompressionOutput((uint8_t*)bufout.data(), bufSize, last);
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
