#include <ctype.h>
#include <stdio.h>
// #include "GzipZlib.hpp"
// #include "ThreadSafeFIFO.hpp"
#include "Stream.hpp"
// #include "GzipZlibCompression.hpp"
// #include "GzipZlibDecompression.hpp"
#include "GzipZlibCompress.hpp"
#include "GzipZlibDecompress.hpp"
#include "LibGzip.hpp"

// #include "gzipApp.hpp"
// #include "gzipOCLHost.hpp"
// #include <memory>

#include "Helper.hpp"
#include <fstream>

namespace testGzip{

// void testGzipSimple(){
// 	std::vector<uint8_t> in, out;
// 	readFile("sample/sample.txt", in, out);
// 	std::vector<uint8_t> out2(20*out.size());

//     uint64_t inputSize=in.size(), outputSize=0, outputSize2=0;
// 	bool last=0;

// 	freopen("output/output_gz", "w", stdout);

//     std::cout<<"Start test Gzip"<<std::endl;
// 	std::cout<<"inputSize: "<<inputSize<<std::endl;
// 	std::cout<<"in.size(): "<<in.size()<<std::endl;
// 	std::cout<<"out.size(): "<<out.size()<<std::endl;
//     // outputSize=dataCompression::gzipZlibCompression(in.data(), out.data(), in.size(), "", false);
// 	dataCompression::gzipZlibCompressionInput(in.data(), 1000, false);
// 	dataCompression::gzipZlibCompressionInput(in.data()+1000, 500, false);
// 	dataCompression::gzipZlibCompressionInput(in.data()+1500, inputSize-1500, true);
// 	uint32_t index=dataCompression::writeGzipHeader(out.data());
// 	while(!last){
// 		outputSize=dataCompression::gzipZlibCompressionOutput(out.data()+index, 500, last);
// 		index+=outputSize;
// 	}
// 	// index=dataCompression::writeGzipFooter(out.data()+index, index, in.size());
// 	std::cout<<"last = "<<last<<std::endl;
// 	std::cout<<"index = "<<index<<std::endl;
// 	std::cout<<"------------------------"<<std::endl;
// 	// outputSize2=dataCompression::gzipZlibDecompression(out.data(), out2.data(), index);
// 	dataCompression::gzipZlibDecompressionInput(out.data(), 500, false);
// 	dataCompression::gzipZlibDecompressionInput(out.data()+500, 300, false);
// 	dataCompression::gzipZlibDecompressionInput(out.data()+800, index-800, true);
// 	last=0;
// 	while(!last) outputSize2+=dataCompression::gzipZlibDecompressionOutput(out2.data()+outputSize2, 800, last);
// 	std::cout<<"last = "<<last<<std::endl;
// 	hexdump(out2.data(), outputSize2);
//     std::cout<<"End test Gzip"<<std::endl;
// }

// void testGzipCompress(){
// 	const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
// 	ofile.open("sample/dt_1G.txt.gz", std::ios::binary);
// 	// ifile.open("sample/sample.txt", std::ios::binary);
// 	// ofile.open("sample/sample.txt.gz", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	uint32_t idx=dataCompression::writeGzipHeader((uint8_t*)bufout.data());
// 	ofile.write(bufout.data(), idx);
// 	std::cout<<"write a "<<idx<<" Bytes header"<<std::endl;

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);
// 			dataCompression::gzipZlibCompressionInput((uint8_t*)buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			uint32_t outputSize=dataCompression::gzipZlibCompressionOutput((uint8_t*)bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
// 			// hexdump(bufout.data(), outputSize);
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	input.join();
// 	output.join();

// 	idx=dataCompression::writeGzipFooter((uint8_t*)bufout.data(), fileSize);
// 	ofile.write(bufout.data(), idx);
// 	std::cout<<"write a "<<idx<<" Bytes footer"<<std::endl;

// 	std::cout<<"test successfully"<<std::endl;
// }

// void testGzipCompress2(){
// 	const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
// 	ofile.open("sample/dt_1G_1.txt.gz", std::ios::binary);
// 	// ifile.open("sample/sample.txt", std::ios::binary);
// 	// ofile.open("sample/sample_1.txt.gz", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	GzipZlibCompressWorkshop workshop(false);
// 	ByteStream &inputFIFO=workshop.getInputStream();
// 	ByteStream &outputFIFO=workshop.getOutputStream();

// 	uint32_t idx=dataCompression::writeGzipHeader((uint8_t*)bufout.data());
// 	ofile.write(bufout.data(), idx);
// 	std::cout<<"write a "<<idx<<" Bytes header"<<std::endl;

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);
// 			// dataCompression::gzipZlibCompressionInput((uint8_t*)buf.data(), curSize, last);
// 			inputFIFO.push(buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			// uint32_t outputSize=dataCompression::gzipZlibCompressionOutput((uint8_t*)bufout.data(), bufSize, last);
// 			uint32_t outputSize=outputFIFO.pop(bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
// 			// hexdump(bufout.data(), outputSize);
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	workshop.wait();
// 	std::cout<<"End of workshop.wait();"<<std::endl;

// 	input.join();
// 	std::cout<<"End of input thread"<<std::endl;
// 	output.join();
// 	std::cout<<"End of output thread"<<std::endl;

// 	idx=dataCompression::writeGzipFooter((uint8_t*)bufout.data(), fileSize);
// 	ofile.write(bufout.data(), idx);
// 	std::cout<<"write a "<<idx<<" Bytes footer"<<std::endl;

// 	std::cout<<"test successfully"<<std::endl;
// }

void testGzipCompress3(int argc, char** argv){
	const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
	ofile.open("sample/dt_1G_3.txt.gz", std::ios::binary);
	// ifile.open("sample/sample.txt", std::ios::binary);
	// ofile.open("sample/sample_3.txt.gz", std::ios::binary);
	// ifile.open(argv[1], std::ios::binary);
	// ofile.open(argv[2], std::ios::binary);

	ifile.seekg(0, std::ios_base::end);
	uint64_t fileSize=ifile.tellg();
	ifile.seekg(0, std::ios_base::beg);

	uint32_t idx=data_compression::gzip::writeGzipHeader((uint8_t*)bufout.data());
	ofile.write(bufout.data(), idx);
	std::cout<<"write a "<<idx<<" Bytes header"<<std::endl;

	std::thread input([&]{
		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			bool last=fileSize-i-curSize==0;
			ifile.read(buf.data(), curSize);
			data_compression::gzip::pushGzipCompression(buf.data(), curSize, i==0, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=data_compression::gzip::popGzipCompression(bufout.data(), bufSize, last);
			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
			// hexdump(bufout.data(), outputSize);
			ofile.write(bufout.data(), outputSize);
		}while(!last);
	});

	input.join();
	std::cout<<"End of input thread"<<std::endl;
	output.join();
	std::cout<<"End of output thread"<<std::endl;

	idx=data_compression::gzip::writeGzipFooter((uint8_t*)bufout.data(), fileSize);
	ofile.write(bufout.data(), idx);
	std::cout<<"write a "<<idx<<" Bytes footer"<<std::endl;

	std::cout<<"test successfully"<<std::endl;
}

// void testGzipDecompress(){
// 	const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("sample/dt_1G_1.txt.gz", std::ios::binary);
// 	ofile.open("sample/dt_1G_1.txt.gz.ori", std::ios::binary);
// 	// ifile.open("sample/sample_1.txt.gz", std::ios::binary);
// 	// ofile.open("sample/sample_1.txt.gz.ori", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);
// 			dataCompression::gzipZlibDecompressionInput((uint8_t*)buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			uint32_t outputSize=dataCompression::gzipZlibDecompressionOutput((uint8_t*)bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	input.join();
// 	output.join();

// 	std::cout<<"test successfully"<<std::endl;
// }

// void testGzipDecompress2(){
// 	const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("sample/dt_1G_1.txt.gz", std::ios::binary);
// 	ofile.open("sample/dt_1G_1.txt.gz.ori", std::ios::binary);
// 	// ifile.open("sample/sample_1.txt.gz", std::ios::binary);
// 	// ofile.open("sample/sample_1.txt.gz.ori", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	GzipZlibDecompressWorkshop workshop;
// 	ByteStream &inputFIFO=workshop.getInputStream();
// 	ByteStream &outputFIFO=workshop.getOutputStream();

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);
// 			// dataCompression::gzipZlibDecompressionInput((uint8_t*)buf.data(), curSize, last);
// 			inputFIFO.push(buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			// uint32_t outputSize=dataCompression::gzipZlibDecompressionOutput((uint8_t*)bufout.data(), bufSize, last);
// 			uint32_t outputSize=outputFIFO.pop(bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	workshop.wait();

// 	input.join();
// 	output.join();

// 	std::cout<<"test successfully"<<std::endl;
// }

void testGzipDecompress3(int argc, char** argv){
	const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	ifile.open("sample/dt_1G_3.txt.gz", std::ios::binary);
	ofile.open("sample/dt_1G_3.txt.gz.ori", std::ios::binary);
	// ifile.open("sample/sample_1.txt.gz", std::ios::binary);
	// ofile.open("sample/sample_1.txt.gz.ori", std::ios::binary);
	// ifile.open(argv[1], std::ios::binary);
	// ofile.open(argv[2], std::ios::binary);

	ifile.seekg(0, std::ios_base::end);
	uint64_t fileSize=ifile.tellg();
	ifile.seekg(0, std::ios_base::beg);

	std::thread input([&]{
		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			bool last=fileSize-i-curSize==0;
			ifile.read(buf.data(), curSize);
			data_compression::gzip::pushGzipDecompression(buf.data(), curSize, i==0, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=data_compression::gzip::popGzipDecompression(bufout.data(), bufSize, last);
			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
			ofile.write(bufout.data(), outputSize);
		}while(!last);
	});

	input.join();
	output.join();

	std::cout<<"test successfully"<<std::endl;
}

// void testZlib(){
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

// void testXilinxGzipZlib(int argc, char** argv){
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

int main(int argc, char** argv){
	testGzip::testGzipCompress3(argc, argv);
	testGzip::testGzipDecompress3(argc, argv);
}