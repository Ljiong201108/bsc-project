#pragma once

#include <map>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>

#include "Util.hpp"

const std::string xclbinFileName="test.xclbin";

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

    static Application& getInstance();
    ~Application();

    //getters
    cl_int &getErr();
    cl::Device &getDevice();
    cl::Program &getProgram();
    cl::Context &getContext();

    //helper functions
    void get_xilinx_devices();
    void read_binary_file(const std::string &xclbin_file_name);
};