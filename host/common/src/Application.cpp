#include "Application.hpp"

void Application::getXilinxDevices(){
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

void Application::loadBinaryFile(const std::string &&xclbin_file_name){
    if (access(xclbin_file_name.c_str(), R_OK)!=0){
        printf("ERROR: %s xclbin not available please build\n", xclbin_file_name.c_str());
        exit(EXIT_FAILURE);
    }
    //Loading XCL Bin into char buffer
    std::ifstream bin_file(xclbin_file_name.c_str(), std::ifstream::binary);
    bin_file.seekg(0, bin_file.end);
    m_xclbinBufferSize = bin_file.tellg();
    bin_file.seekg(0, bin_file.beg);
    if(!m_xclbinBuffer) m_xclbinBuffer = new char[m_xclbinBufferSize];
    else{
        delete[] m_xclbinBuffer;
        m_xclbinBuffer = new char[m_xclbinBufferSize];
    }
    bin_file.read(m_xclbinBuffer, m_xclbinBufferSize);

    cl::Program::Binaries bins{{m_xclbinBuffer, m_xclbinBufferSize}};
    OCL_CHECK(m_err, m_program=std::make_unique<cl::Program>(m_context, (std::vector<cl::Device>){m_device}, bins, (std::vector<cl_int>*)NULL, &m_err))
}

Application::Application(){
    getXilinxDevices();
    OCL_CHECK(m_err, m_context=cl::Context(m_device, NULL, NULL, NULL, &m_err))
    m_lib=Lib::EMPTY;
    m_xclbinBuffer=NULL;
}

Application::~Application(){
    delete[] m_xclbinBuffer;
}

Application& Application::getInstance(){
    static Application instance;
    return instance;
}

cl::Device& Application::getDevice(){
    return getInstance().m_device;
}

cl::Context& Application::getContext(){
    return getInstance().m_context;
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