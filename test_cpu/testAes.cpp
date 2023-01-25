#include "openssl/aes.h"
#include "bits/stdc++.h"
#include "Timer.hpp"

inline void testAes(char** argv){
	uint64_t key[]={0x66666666, 0x77777777}, iv[]={0x11111111, 0x22222222};
	
	std::vector<uint32_t> blockSizes;

	std::ofstream summary;
	summary.open("output/summary.txt", std::ios::binary);

	{
		Timer::reset();
		std::cout<<"Starting testing enc!"<<std::endl;
		Timer::startTotalTimer();

		const uint64_t bufSize=64*1024*1024;
		std::vector<char> buf(bufSize), bufout(bufSize);
		std::ofstream ofile;
		std::ifstream ifile;

		ifile.open("sample/dataset_1G.txt", std::ios::binary);
		ofile.open("sample/dt_1G.txt.enc", std::ios::binary);
		// ifile.open("sample/sample.txt", std::ios::binary);
		// ofile.open("sample/sample.txt.enc", std::ios::binary);
		// ifile.open(argv[1], std::ios::binary);
		// ofile.open(argv[2], std::ios::binary);

		ifile.seekg(0, std::ios_base::end);
		uint64_t fileSize=ifile.tellg();
		ifile.seekg(0, std::ios_base::beg);

		std::cout<<fileSize<<std::endl;

		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			uint32_t enc_size=16*(curSize/16+(curSize%16!=0));
			blockSizes.push_back(curSize);
			ifile.read(buf.data(), curSize);
			if(curSize<bufSize) memset(buf.data()+curSize, 0, enc_size-curSize);
			if(i==0) security::aes::aesCtrEncrypt(buf.data(), bufout.data(), enc_size, key, 16, iv);
			else security::aes::aesCtrEncrypt(buf.data(), bufout.data(), enc_size, key, 16);
			ofile.write(bufout.data(), enc_size);
			std::cout<<i/bufSize<<": host processed a "<<enc_size<<" Bytes block"<<std::endl;
		}

		ifile.close();
		ofile.close();

		Timer::endTotalTimer();
		summary<<"The Encryption used "<<Timer::totalTime.count()<<" ns in total"<<std::endl;
		summary<<"The time for compute is "<<Timer::computeTime.count()<<" ns"<<std::endl;
		summary<<"The IO time is "<<Timer::ioTime.count()<<" ns"<<std::endl;
		std::cout<<"Finished testing successfully!"<<std::endl;
	}

	{
		Timer::reset();
		std::cout<<"Starting testing dec!"<<std::endl;
		Timer::startTotalTimer();

		const uint64_t bufSize=64*1024*1024;
		std::vector<char> buf(bufSize), bufout(bufSize);
		std::ofstream ofile;
		std::ifstream ifile;

		ifile.open("sample/dt_1G.txt.enc", std::ios::binary);
		ofile.open("sample/dt_1G.txt.enc.dec", std::ios::binary);
		// ifile.open("sample/sample.txt.enc", std::ios::binary);
		// ofile.open("sample/sample.txt.enc.dec", std::ios::binary);
		// ifile.open(argv[2], std::ios::binary);
		// ofile.open(argv[3], std::ios::binary);

		ifile.seekg(0, std::ios_base::end);
		uint64_t fileSize=ifile.tellg();
		ifile.seekg(0, std::ios_base::beg);

		std::cout<<fileSize<<std::endl;

		for(uint64_t i=0;i<fileSize;i+=bufSize){
			uint32_t curSize=(fileSize-i>bufSize?bufSize:fileSize-i);
			blockSizes.push_back(curSize);
			ifile.read(buf.data(), curSize);
			// if(curSize<bufSize) memset(buf.data()+curSize, 0, bufSize-curSize);
			if(i==0) security::aes::aesCtrDecrypt(buf.data(), bufout.data(), curSize, key, 16, iv);
			else security::aes::aesCtrDecrypt(buf.data(), bufout.data(), curSize, key, 16);
			ofile.write(bufout.data(), blockSizes[i/bufSize]);
			std::cout<<i/bufSize<<": host processed a "<<curSize<<" Bytes block"<<std::endl;
		}

		ifile.close();
		ofile.close();

		Timer::endTotalTimer();
		summary<<"The Decryption used "<<Timer::totalTime.count()<<" ns in total"<<std::endl;
		summary<<"The time for compute is "<<Timer::computeTime.count()<<" ns"<<std::endl;
		summary<<"The IO time is "<<Timer::ioTime.count()<<" ns"<<std::endl;
		std::cout<<"Finished testing successfully!"<<std::endl;
	}

	summary.close();
}