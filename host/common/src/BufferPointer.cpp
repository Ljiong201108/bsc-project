#include "BufferPointer.hpp"

BufferPointer::BufferPointer() : Pointer<cl::Buffer>(){}

BufferPointer &BufferPointer::create(const cl::Context& context, cl_mem_flags flags, size_t size, void* host_ptr){
    cl_int err;
    OCL_CHECK(err, build(context, flags, size, host_ptr, &err))
    return *this;
}
