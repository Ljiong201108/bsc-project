#pragma once

#include "Util.hpp"
#include "Pointer.hpp"

class KernelPointer : public Pointer<cl::Kernel>{
public:
    KernelPointer();
    KernelPointer(const cl::Program& program, const std::string &name);
    KernelPointer &create(const cl::Program& program, const std::string &name);
};