#include "KernelPointer.hpp"

KernelPointer::KernelPointer() : Pointer<cl::Kernel>(){}

KernelPointer::KernelPointer(const cl::Program& program, const std::string &name){
    cl_int err;
    OCL_CHECK(err, build(program, name.c_str(), &err))
}

KernelPointer &KernelPointer::create(const cl::Program& program, const std::string &name){
    cl_int err;
    OCL_CHECK(err, build(program, name.c_str(), &err))
    return *this;
}
