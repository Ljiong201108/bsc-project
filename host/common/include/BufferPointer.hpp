#pragma once

#include "Util.hpp"
#include "Pointer.hpp"

class BufferPointer : public Pointer<cl::Buffer>{
public:
    BufferPointer();
    BufferPointer(const cl::Context& context, cl_mem_flags flags, size_t size, void* host_ptr = NULL);
    BufferPointer &create(const cl::Context& context, cl_mem_flags flags, size_t size, void* host_ptr = NULL);
};