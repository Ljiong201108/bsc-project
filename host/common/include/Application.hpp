#pragma once

#include <map>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>

#define CL_HPP_CL_1_2_DEFAULT_BUILD                                    
#define CL_HPP_TARGET_OPENCL_VERSION 120                               
#define CL_HPP_MINIMUM_OPENCL_VERSION 120                            
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS      
#include <CL/cl2.hpp>

#include "Util.hpp"

class Application{
private:
    cl_int m_err;
    cl::Device m_device;
    cl::Context m_context;
    cl::Program m_program;
    unsigned m_xclbinBufferSize;
    char *m_xclbinBuffer;
    
public:
    Application(const std::string &binaryFileName);
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