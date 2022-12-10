#pragma once

#include <map>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>

#include "Util.hpp"

// const std::string xclbinFileName="fpga.xclbin";

enum class Lib{
    securityLib, dataCompressionLib
};

const std::string xclbinFileNames[]={"security.xclbin", "fpga.xclbin"};

class Application{
private:
    cl_int m_err;
    cl::Device m_device;
    cl::Context m_context;
    cl::Program m_program;
    unsigned m_xclbinBufferSize;
    char *m_xclbinBuffer;

    Application(const std::string &binaryFileName);
    
public:
    Application(const Application&)=delete;
    Application& operator=(const Application&)=delete;

    template<Lib L>
    static Application& getInstance();

    ~Application();

    //getters
    template<Lib L>
    static cl::Device &getDevice();

    template<Lib L>
    static cl::Program &getProgram();

    template<Lib L>
    static cl::Context &getContext();

    //helper functions
    void get_xilinx_devices();
    void read_binary_file(const std::string &xclbin_file_name);
};

template<Lib L>
Application& Application::getInstance(){
    static Application instance(xclbinFileNames[(int)L]);
    return instance;
}

template<Lib L>
cl::Device &Application::getDevice(){
    return getInstance<L>().m_device;
}

template<Lib L>
cl::Program &Application::getProgram(){
    return getInstance<L>().m_program;
}

template<Lib L>
cl::Context &Application::getContext(){
    return getInstance<L>().m_context;
}