#include "KernelPointer.hpp"

KernelPointer::KernelPointer() : Pointer<cl::Kernel>(){}

KernelPointer &KernelPointer::create(const cl::Program& program, const std::string &name){
    cl_int err;
    OCL_CHECK(err, build(program, name.c_str(), &err))
    return *this;
}

// KernelPointer::KernelPointer(Application &app, const std::string &name) : Pointer<cl::Kernel>(app, app.getProgram(), name.c_str(), &app.getErr()){}