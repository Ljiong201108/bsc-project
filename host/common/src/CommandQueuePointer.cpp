#include "CommandQueuePointer.hpp"

CommandQueuePointer::CommandQueuePointer() : Pointer<cl::CommandQueue>(){}

CommandQueuePointer::CommandQueuePointer(const cl::Context& context, const cl::Device& device, cl_command_queue_properties properties) : CommandQueuePointer(){
    cl_int err;
    OCL_CHECK(err, build(context, device, properties, &err))
}

CommandQueuePointer &CommandQueuePointer::create(const cl::Context& context, const cl::Device& device, cl_command_queue_properties properties){
    cl_int err;
    OCL_CHECK(err, build(context, device, properties, &err))
    return *this;
}
