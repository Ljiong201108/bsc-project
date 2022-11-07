#include "Application.hpp"

void Application::get_xilinx_devices(){
    std::vector<cl::Platform> platforms;
    OCL_CHECK(m_err, m_err = cl::Platform::get(&platforms))
    cl::Platform platform;
    for (size_t i=0;i<platforms.size();i++){
        platform = platforms[i];
        OCL_CHECK(m_err, std::string platformName=platform.getInfo<CL_PLATFORM_NAME>(&m_err))
        if (platformName=="Xilinx"){
            #ifdef DEBUG
            std::cout<<"INFO: Found Xilinx Platform"<<std::endl;
            #endif
            //Getting ACCELERATOR Devices and selecting 1st such device
            std::vector<cl::Device> devices;
            OCL_CHECK(m_err, m_err = platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices))
            devices.resize(1);
            m_device = devices[0];
            return;
        }
    }

    std::cout << "ERROR: Failed to find Xilinx platform" << std::endl;
    exit(EXIT_FAILURE);
}

void Application::read_binary_file(const std::string &xclbin_file_name){
    if (access(xclbin_file_name.c_str(), R_OK)!=0){
        printf("ERROR: %s xclbin not available please build\n", xclbin_file_name.c_str());
        exit(EXIT_FAILURE);
    }
    //Loading XCL Bin into char buffer
    #ifdef DEBUG
    std::cout<<"INFO: Loading '"<<xclbin_file_name<<"'\n";
    #endif
    std::ifstream bin_file(xclbin_file_name.c_str(), std::ifstream::binary);
    bin_file.seekg(0, bin_file.end);
    m_xclbinBufferSize = bin_file.tellg();
    bin_file.seekg(0, bin_file.beg);
    m_xclbinBuffer = new char[m_xclbinBufferSize];
    bin_file.read(m_xclbinBuffer, m_xclbinBufferSize);
}

Application::Application(const std::string &binaryFileName){
    get_xilinx_devices();
    OCL_CHECK(m_err, m_context=cl::Context(m_device, NULL, NULL, NULL, &m_err))
    read_binary_file(binaryFileName);
    cl::Program::Binaries bins{{m_xclbinBuffer, m_xclbinBufferSize}};
    OCL_CHECK(m_err, m_program=cl::Program(m_context, {m_device}, bins, NULL, &m_err))
}

Application::~Application(){
    #ifdef DEBUG
    std::cout<<"INFO: Deconstructing Application with xclbin '"<<xclbin_file_name<<"'\n";
    #endif
    delete[] m_xclbinBuffer;
}

cl_int &Application::getErr(){
    return m_err;
}

cl::Device &Application::getDevice(){
    return m_device;
}

cl::Program &Application::getProgram(){
    return m_program;
}

cl::Context &Application::getContext(){
    return m_context;
}

// CommandQueuePointer &Application::getOrCreateCommandQueue(const std::string &identifier, cl_command_queue_properties properties){
//     OCL_CHECK(m_err, auto ret=m_commandQueues.emplace(*this, properties, &m_err))
//     return (*ret.first).second;
// }
    
// KernelPointer &Application::getOrCreateKernel(const std::string &identifier, const std::string &name){
//     OCL_CHECK(m_err, auto ret=m_commandQueues.emplace(*this, m_program, name.c_str(), &m_err))
//     return ret;
// }

// BufferPointer &Application::getOrCreateBuffer(cl_mem_flags flags, size_t size, void* host_ptr, std::list<cl::Buffer>::iterator *iterator){
//     OCL_CHECK(m_err, auto &ret=m_buffers.emplace_back(m_context, flags, size, host_ptr, &m_err))
//     if(iterator) *iterator=--m_buffers.end();
//     return ret;
// }