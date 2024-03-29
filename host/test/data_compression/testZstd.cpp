#ifdef XILINX
#include "zstdApp.hpp"
#include "zstdOCLHost.hpp"
#include <memory>
#else
#include "Zstd.hpp"
#endif

#include "Helper.hpp"
#include "ZstdCompress.hpp"
#include "ZstdDecompress.hpp"
#include "LibZstd.hpp"

namespace testZstd{
// void testZstdSimple(int argc, char** argv){
// #ifdef XILINX
//     bool enable_profile = false;
//     compressBase::State flow = compressBase::BOTH;
//     compressBase::Level lflow = compressBase::SEQ;
//     // Driver class object
//     zstdApp d(argc, argv, lflow);

//     // Design class object creating and constructor invocation
//     std::unique_ptr<zstdOCLHost> zstd(new zstdOCLHost(flow, d.getXclbin(), d.getDeviceId()));

//     // Run API to launch the compress or decompress engine
//     d.run(zstd.get(), enable_profile);
// #else
//     std::vector<uint8_t> in, out;
// 	// readFile("sample.txt", in, out);
//     readFile("sample.txt", in, out);
// 	std::vector<uint8_t> out2;
//     out2.resize(20 * in.size());

//     uint64_t inputSize=in.size(), outputSize=0, outputSize2=0;
//     bool last;

//     freopen("output_zstd", "w", stdout);

//     std::cout<<"Start test Zstd"<<std::endl;
//     std::cout<<"Original: "<<std::endl;
//     std::cout<<"in.size(): "<<in.size()<<std::endl;
//     std::cout<<"out.size(): "<<out.size()<<std::endl;
//     // outputSize=dataCompression::zstdCompress(in.data(), out.data(), inputSize);
//     dataCompression::zstdCompressionInput(in.data(), inputSize, true);
//     outputSize+=dataCompression::zstdCompressionOutput(out.data(), out.size(), last);
//     std::cout<<"Compressed: "<<std::endl;
//     hexdump(out.data(), outputSize);
//     std::cout<<"------------------------"<<std::endl;
// 	// outputSize2=dataCompression::zstdDecompress(out.data(), out2.data(), outputSize);
//     dataCompression::zstdDecompressionInput(out.data(), outputSize, true);
//     outputSize2=dataCompression::zstdDecompressionOutput(out2.data(), out2.size(), last);
//     std::cout<<"last: "<<last<<std::endl;
//     hexdump(out2.data(), outputSize2);
//     std::cout<<"End test Zstd"<<std::endl;
// #endif
// }

// void testZstdCompress(){
//     freopen("output/zstd_output_compress.txt", "w", stdout);

//     const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	// ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
// 	// ofile.open("sample/dt_1G.txt.zst", std::ios::binary);
// 	ifile.open("sample/sample.txt", std::ios::binary);
// 	ofile.open("sample/sample.txt.zst", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);
// 			dataCompression::zstdCompressionInput((uint8_t*)buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			uint32_t outputSize=dataCompression::zstdCompressionOutput((uint8_t*)bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
// 			// hexdump(bufout.data(), outputSize);
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	input.join();
// 	output.join();

// 	std::cout<<"zstd compress successfully"<<std::endl;

//     ifile.close();
//     ofile.close();
// }

// void testZstdCompress2(){
//     freopen("output/zstd_output_compress.txt", "w", stdout);

//     const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("/share/xilinx/dt_1G.txt", std::ios::binary);
// 	ofile.open("sample/dt_1G_2.txt.zst", std::ios::binary);
// 	// ifile.open("sample/sample.txt", std::ios::binary);
// 	// ofile.open("sample/sample_2.txt.zst", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	ZstdCompressWorkshop workshop;
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
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
// 			// hexdump(bufout.data(), outputSize);
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	workshop.wait();
// 	input.join();
// 	output.join();

// 	std::cout<<"zstd compress successfully"<<std::endl;

//     ifile.close();
//     ofile.close();
// }

void testZstdCompress3(int argc, char** argv){
    // freopen("output/zstd_output_compress3.txt", "w", stdout);

	Timer::reset();
    const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	ifile.open(argv[1], std::ios::binary);
	ofile.open("sample/compress", std::ios::binary);
	// ifile.open("sample/sample.txt", std::ios::binary);
	// ofile.open("sample/sample_2.txt.zst", std::ios::binary);

	std::ofstream summary;
	summary.open("output/summary.txt", std::ios::binary | std::ios::app);

	ifile.seekg(0, std::ios_base::end);
	uint64_t fileSize=ifile.tellg();
	ifile.seekg(0, std::ios_base::beg);

	Application::getProgram<Lib::ZSTD>();

	Timer::startTotalTimer();

	std::thread input([&]{
		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			bool last=fileSize-i-curSize==0;
			ifile.read(buf.data(), curSize);
			data_compression::zstd::pushZstdCompression(buf.data(), curSize, i==0, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=data_compression::zstd::popZstdCompression(bufout.data(), bufSize, last);
			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
			// hexdump(bufout.data(), outputSize);
			ofile.write(bufout.data(), outputSize);
		}while(!last);
	});

	input.join();
	output.join();

	Timer::endTotalTimer();
	std::cout<<"test successfully"<<std::endl;

	summary<<"Total Time Compress["<<argv[1]<<"]: "<<Timer::totalTime.count()<<std::endl;

    // ifile.close();
    // ofile.close();
}

// void testZstdDecompress(){
//     freopen("output/zstd_output_decompress.txt", "w", stdout);

// 	const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("sample/dt_1G.txt.zst", std::ios::binary);
// 	ofile.open("sample/dt_1G.txt.zst.ori", std::ios::binary);
// 	// ifile.open("sample/sample.txt.zst", std::ios::binary);
// 	// ofile.open("sample/sample.txt.zst.ori", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	std::thread input([&]{
// 		for(uint64_t i=0;i<fileSize;i+=bufSize){
// 			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
// 			bool last=fileSize-i-curSize==0;
// 			ifile.read(buf.data(), curSize);

//             if(i==0){
//                 hexdump(buf.data(), 20*1024);
//             }

// 			dataCompression::zstdDecompressionInput((uint8_t*)buf.data(), curSize, last);
// 			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
// 		}
// 	});

// 	std::thread output([&]{
// 		bool last;
// 		do{
// 			uint32_t outputSize=dataCompression::zstdDecompressionOutput((uint8_t*)bufout.data(), bufSize, last);
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	input.join();
// 	output.join();

// 	std::cout<<"zstd decompress successfully"<<std::endl;

//     ifile.close();
//     ofile.close();
// }

// void testZstdDecompress2(){
//     freopen("output/zstd_output_decompress2.txt", "w", stdout);

// 	const uint64_t bufSize=64*1024*1024;
// 	std::vector<char> buf(bufSize), bufout(bufSize);
// 	std::ofstream ofile;
//     std::ifstream ifile;

// 	ifile.open("sample/dt_1G_2.txt.zst", std::ios::binary);
// 	ofile.open("sample/dt_1G_2.txt.zst.ori", std::ios::binary);
// 	// ifile.open("sample/sample_2.txt.zst", std::ios::binary);
// 	// ofile.open("sample/sample_2.txt.zst.ori", std::ios::binary);

// 	ifile.seekg(0, std::ios_base::end);
// 	uint64_t fileSize=ifile.tellg();
// 	ifile.seekg(0, std::ios_base::beg);

// 	ZstdDecompressWorkshop workshop;
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
// 			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
// 			ofile.write(bufout.data(), outputSize);
// 		}while(!last);
// 	});

// 	workshop.wait();

// 	input.join();
// 	output.join();

// 	std::cout<<"zstd decompress successfully"<<std::endl;

//     ifile.close();
//     ofile.close();
// }

void testZstdDecompress3(){
    // freopen("output/zstd_output_decompress3.txt", "w", stdout);
	Timer::reset();

	const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize), bufout(bufSize);
	std::ofstream ofile;
    std::ifstream ifile;

	ifile.open("sample/compress", std::ios::binary);
	ofile.open("sample/decompress", std::ios::binary);
	// ifile.open("sample/sample_2.txt.zst", std::ios::binary);
	// ofile.open("sample/sample_2.txt.zst.ori", std::ios::binary);

	std::ofstream summary;
	summary.open("output/summary.txt", std::ios::binary | std::ios::app);

	ifile.seekg(0, std::ios_base::end);
	uint64_t fileSize=ifile.tellg();
	ifile.seekg(0, std::ios_base::beg);

	Application::getProgram<Lib::ZSTD>();

	Timer::startTotalTimer();

	std::thread input([&]{
		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			bool last=fileSize-i-curSize==0;
			ifile.read(buf.data(), curSize);

			data_compression::zstd::pushZstdDecompression(buf.data(), curSize, i==0, last);
			std::cout<<"host write a "<<curSize<<" Bytes block into FIFO"<<std::endl;
		}
	});

	std::thread output([&]{
		bool last;
		do{
			uint32_t outputSize=data_compression::zstd::popZstdDecompression(bufout.data(), bufSize, last);
			std::cout<<"host read a "<<outputSize<<" Bytes block from FIFO"<<std::endl;
			ofile.write(bufout.data(), outputSize);
		}while(!last);
	});

	input.join();
	output.join();

	Timer::endTotalTimer();
	std::cout<<"snappy decompress successfully"<<std::endl;

	summary<<"Total Time Decompress: "<<Timer::totalTime.count()<<std::endl;
	summary<<"Total process time: "<<Timer::anaTime.count()<<std::endl;
	summary<<"FPGA time: "<<Timer::fpgaInitTime.count()<<std::endl;
	summary<<std::endl;

    ifile.close();
    ofile.close();
}
} //testZstd

int main(int argc, char** argv){
	// testZstd::testZstdCompress();
	testZstd::testZstdCompress3(argc, argv);
	// testZstd::testZstdDecompress();
	testZstd::testZstdDecompress3();
	exit(0);
}