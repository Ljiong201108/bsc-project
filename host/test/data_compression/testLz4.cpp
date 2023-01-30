#ifdef XILINX
#include "lz4App.hpp"
#include "lz4OCLHost.hpp"
#include <memory>
#else
#include "Lz4.hpp"
#endif

#include "Helper.hpp"
#include "Lz4Compress.hpp"
#include "Lz4Decompress.hpp"
#include "LibLz4.hpp"

namespace testLz4{
// void testLz4Simple(int argc, char** argv){
// #ifdef XILINX
//     // bool enable_profile = false;
//     // bool lz4_stream = false;
//     // compressBase::State flow = compressBase::BOTH;
//     // compressBase::Level lflow = compressBase::SEQ;
//     // // Driver class object
//     // lz4App d(argc, argv, lflow);

//     // // Design class object creating and constructor invocation
//     // std::unique_ptr<lz4OCLHost> lz4(new lz4OCLHost(flow, d.getXclbin(), d.getDeviceId(), d.getBlockSize(), lz4_stream));

//     // // Run API to launch the compress or decompress engine
//     // d.run(lz4.get(), enable_profile);

//     bool enable_profile = false;
//     bool lz4_stream = true;
//     compressBase::State flow = compressBase::BOTH;
//     compressBase::Level lflow = compressBase::SEQ;
//     // Driver class object
//     lz4App d(argc, argv, lflow);

//     // Design class object creating and constructor invocation
//     std::unique_ptr<lz4OCLHost> lz4(new lz4OCLHost(flow, d.getXclbin(), d.getDeviceId(), d.getBlockSize(), lz4_stream));

//     // Run API to launch the compress or decompress engine
//     d.run(lz4.get(), enable_profile);
// #else
//     std::vector<uint8_t> in, out;
// 	readFile("sample.txt", in, out);
// 	std::vector<uint8_t> out2;
//     out2.resize(20 * in.size());

//     uint64_t inputSize=in.size(), outputSize=0, outputSize2=0;
//     bool last;

//     freopen("output_lz4", "w", stdout);

//     std::cout<<"Start test Lz4"<<std::endl;
//     std::cout<<"Original: "<<std::endl;
//     hexdump(in.data(), inputSize);
//     // outputSize=dataCompression::lz4Compress(in.data(), out.data(), inputSize);
//     outputSize+=dataCompression::writeLz4Header(out.data(), inputSize);
//     dataCompression::lz4CompressionInput(in.data(), inputSize, true);
//     outputSize+=dataCompression::lz4CompressionOutput(out.data()+outputSize, out.size(), last);
//     outputSize+=dataCompression::writeLz4Footer(out.data()+outputSize);
//     std::cout<<"Compressed: ["<<outputSize<<" Bytes]"<<std::endl;
//     hexdump(out.data(), outputSize);
// 	std::cout<<"------------------------"<<std::endl;
//     dataCompression::lz4DecompressionInput(out.data(), outputSize, true);
//     outputSize2=dataCompression::lz4DecompressionOutput(out2.data(), out2.size(), last);
//     // outputSize2=dataCompression::lz4Decompress(out.data(), out2.data(), outputSize, out2.size());
//     std::cout<<"Decompressed: "<<std::endl;
//     hexdump(out2.data(), outputSize2);
//     std::cout<<"End test Lz4"<<std::endl;
// #endif
// }

// void testLz4Compress(){
//     freopen("output/lz4_output_compress.txt", "w", stdout);

//     const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
// 	ofile.open("sample/dt_1G.txt.lz4", std::ios::binary);
// 	// ifile.open("sample/sample.txt", std::ios::binary);
// 	// ofile.open("sample/sample.txt.lz4", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	uint32_t idx=dataCompression::writeLz4Header((uint8_t*)bufout.data(), fileSize);
// 	ofile.write(bufout.data(), idx);
// 	std::cout<<"write a "<<idx<<" Bytes header"<<std::endl;

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);
// 			dataCompression::lz4CompressionInput((uint8_t*)buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			uint32_t outputSize=dataCompression::lz4CompressionOutput((uint8_t*)bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
// 			// hexdump(bufout.data(), outputSize);
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	input.join();
// 	output.join();

//     idx=dataCompression::writeLz4Footer((uint8_t*)bufout.data());
// 	ofile.write(bufout.data(), idx);
// 	std::cout<<"write a "<<idx<<" Bytes footer"<<std::endl;

// 	std::cout<<"lz4 compress successfully"<<std::endl;

//     ifile.close();
//     ofile.close();
// }

// void testLz4Compress2(){
//     freopen("output/lz4_output_compress_2.txt", "w", stdout);

//     const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
// 	ofile.open("sample/dt_1G_2.txt.lz4", std::ios::binary);
// 	// ifile.open("sample/sample.txt", std::ios::binary);
// 	// ofile.open("sample/sample_2.txt.lz4", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	uint32_t idx=dataCompression::writeLz4Header((uint8_t*)bufout.data(), fileSize);
// 	ofile.write(bufout.data(), idx);
// 	std::cout<<"write a "<<idx<<" Bytes header"<<std::endl;

// 	Lz4CompressWorkshop workshop;
// 	ByteStream &inputStream=workshop.getInputStream();
// 	ByteStream &outputStream=workshop.getOutputStream();

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);
// 			inputStream.push(buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			uint32_t outputSize=outputStream.pop(bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO, last="<<last<<std::endl;
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	workshop.wait();

// 	input.join();
// 	output.join();

//     idx=dataCompression::writeLz4Footer((uint8_t*)bufout.data());
// 	ofile.write(bufout.data(), idx);
// 	std::cout<<"write a "<<idx<<" Bytes footer"<<std::endl;

// 	std::cout<<"lz4 compress successfully"<<std::endl;

//     ifile.close();
//     ofile.close();
// }

void testLz4Compress3(int argc, char** argv){
    // freopen("output/lz4_output_compress_2.txt", "w", stdout);

	Timer::reset();
    const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	// ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
	ifile.open(argv[1], std::ios::binary);
	ofile.open("sample/compress", std::ios::binary);
	// ifile.open("sample/sample.txt", std::ios::binary);
	// ofile.open("sample/sample_2.txt.lz4", std::ios::binary);

	std::ofstream summary;
	summary.open("output/summary.txt", std::ios::binary | std::ios::app);

	ifile.seekg(0, std::ios_base::end);
	uint64_t fileSize=ifile.tellg();
	ifile.seekg(0, std::ios_base::beg);

	Application::getProgram<Lib::LZ4_COMPRESSION>();

	Timer::startTotalTimer();

	uint32_t idx=data_compression::lz4::writeLz4Header((uint8_t*)bufout.data(), fileSize);
	ofile.write(bufout.data(), idx);
	std::cout<<"write a "<<idx<<" Bytes header"<<std::endl;

	std::thread input([&]{
		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			bool last=fileSize-i-curSize==0;
			ifile.read(buf.data(), curSize);
			data_compression::lz4::pushLz4Compression(buf.data(), curSize, i==0, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=data_compression::lz4::popLz4Compression(bufout.data(), bufSize, last);
			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO, last="<<last<<std::endl;
			ofile.write(bufout.data(), outputSize);
		}while(!last);
	});

	input.join();
	output.join();

    idx=data_compression::lz4::writeLz4Footer((uint8_t*)bufout.data());
	ofile.write(bufout.data(), idx);
	std::cout<<"write a "<<idx<<" Bytes footer"<<std::endl;

	Timer::endTotalTimer();
	std::cout<<"test successfully"<<std::endl;

	summary<<"Total Time Compress["<<argv[1]<<"]: "<<Timer::totalTime.count()<<std::endl;

    ifile.close();
    ofile.close();
}

// void testLz4Decompress(){
//     freopen("output/lz4_output_decompress.txt", "w", stdout);

// 	const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	// ifile.open("sample/dt_1G.txt.lz4", std::ios::binary);
// 	// ofile.open("sample/dt_1G.txt.lz4.ori", std::ios::binary);
// 	ifile.open("sample/compress", std::ios::binary);
// 	ofile.open("sample/decompress", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);

// 			dataCompression::lz4DecompressionInput((uint8_t*)buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			uint32_t outputSize=dataCompression::lz4DecompressionOutput((uint8_t*)bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO, last="<<last<<std::endl;
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	input.join();
// 	output.join();

// 	std::cout<<"lz4 decompress successfully"<<std::endl;

//     ifile.close();
//     ofile.close();
// }

// void testLz4Decompress2(){
//     freopen("output/lz4_output_decompress_2.txt", "w", stdout);

// 	const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("sample/dt_1G.txt.lz4", std::ios::binary);
// 	ofile.open("sample/dt_1G_2.txt.lz4.ori", std::ios::binary);
// 	// ifile.open("sample/sample.txt.lz4", std::ios::binary);
// 	// ofile.open("sample/sample_2.txt.lz4.ori", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	Lz4DecompressWorkshop workshop;
// 	ByteStream &inputStream=workshop.getInputStream();
// 	ByteStream &outputStream=workshop.getOutputStream();

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);

// 			inputStream.push(buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO, last="<<last<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			uint32_t outputSize=outputStream.pop(bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO, last="<<last<<std::endl;
// 			if(last) ofile.write(bufout.data(), outputSize-1);
// 			else ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	workshop.wait();

// 	std::cout<<"after wait"<<std::endl;

// 	input.join();
// 	output.join();

// 	std::cout<<"lz4 decompress successfully"<<std::endl;

//     ifile.close();
//     ofile.close();
// }

void testLz4Decompress3(int argc, char** argv){
    // freopen("output/lz4_output_decompress_2.txt", "w", stdout);
	Timer::reset();

	const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	// ifile.open("sample/dt_1G_3.txt.lz4", std::ios::binary);
	// ofile.open("sample/dt_1G_3.txt.lz4.ori", std::ios::binary);
	// ifile.open("sample/sample.txt.lz4", std::ios::binary);
	// ofile.open("sample/sample_2.txt.lz4.ori", std::ios::binary);
	ifile.open("sample/compress", std::ios::binary);
	ofile.open("sample/decompress", std::ios::binary);

	std::ofstream summary;
	summary.open("output/summary.txt", std::ios::binary | std::ios::app);

	ifile.seekg(0, std::ios_base::end);
	uint64_t fileSize=ifile.tellg();
	ifile.seekg(0, std::ios_base::beg);

	Application::getProgram<Lib::LZ4_DECOMPRESSION>();

	Timer::startTotalTimer();

	std::thread input([&]{
		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			bool last=fileSize-i-curSize==0;
			ifile.read(buf.data(), curSize);

			data_compression::lz4::pushLz4Decompression(buf.data(), curSize, i==0, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO, last="<<last<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=data_compression::lz4::popLz4Decompression(bufout.data(), bufSize, last);
			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO, last="<<last<<std::endl;
			ofile.write(bufout.data(), outputSize);
		}while(!last);
	});

	input.join();
	output.join();

	Timer::endTotalTimer();
	std::cout<<"lz4 decompress successfully"<<std::endl;

	summary<<"Total Time Decompress: "<<Timer::totalTime.count()<<std::endl;
	summary<<std::endl;

    ifile.close();
    ofile.close();
}
} //testLz4

int main(int argc, char** argv){
	// for(int i=1;i<=3;i++){
		testLz4::testLz4Compress3(argc, argv);
		testLz4::testLz4Decompress3(argc, argv);
	// }
}