#pragma once

#include <map>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <thread>

#include "Util.hpp"

// const std::string xclbinFileName="fpga.xclbin";

enum class Lib{
	EMPTY,
	DES, 
	AES128, 
	AES192, 
	AES256, 
	GZIP_ZLIB_COMPRESSION, 
	GZIP_ZLIB_DECOMPRESSION,
	SNAPPY, 
	LZ4, 
	LZ4_COMPRESSION,
	LZ4_DECOMPRESSION,
	ZSTD,
	PART,
	JOIN,
	AGGR
};

const std::string xclbinFileNames[]={
	"",
	"des.xclbin", 
	"aes128.xclbin", 
	"aes192.xclbin", 
	"aes256.xclbin", 
	"gzip_zlib_compress.xclbin", 
	"gzip_zlib_decompress.xclbin",
	"snappy.xclbin", 
	"lz4.xclbin", 
	"lz4_compress.xclbin",
	"lz4_decompress.xclbin",
	"zstd.xclbin",
	"gqePart.xclbin",
	"gqeJoin.xclbin",
	"gqeAggr.xclbin"
};

class Application{
private:
	cl_int m_err;
	cl::Device m_device;
	cl::Context m_context;
	std::unique_ptr<cl::Program> m_program;
	unsigned m_xclbinBufferSize;
	char *m_xclbinBuffer;
	static std::mutex mtx;

	Lib m_lib;

	Application();
	
public:
	Application(const Application&)=delete;
	Application& operator=(const Application&)=delete;

	static Application& getInstance();

	~Application();

	//getters
	static cl::Device &getDevice();

	static cl::Context &getContext();

	template<Lib L>
	static cl::Program &getProgram();

	//helper functions
	void getXilinxDevices();
	void loadBinaryFile(const std::string &&xclbin_file_name);
};

template<Lib L>
cl::Program& Application::getProgram(){
	static_assert(L!=Lib::EMPTY && "Cannot switch to EMPTY xclbin file!");

	std::unique_lock<std::mutex> lck(Application::mtx);

	if(L!=getInstance().m_lib){
		getInstance().m_program.reset(nullptr);
		getInstance().loadBinaryFile(xclbinPath()+"/"+xclbinFileNames[(int)L]);
		getInstance().m_lib=L;
		std::cout<<"change the xclbin file to "<<xclbinFileNames[(int)L]<<std::endl;
	}

	return *getInstance().m_program;
}
