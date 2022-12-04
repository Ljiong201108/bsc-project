#include "CommandQueuePointer.hpp"

CommandQueuePointer::CommandQueuePointer() : Pointer<cl::CommandQueue>(){}

CommandQueuePointer &CommandQueuePointer::create(const cl::Context& context, const cl::Device& device, cl_command_queue_properties properties){
    cl_int err;
    OCL_CHECK(err, build(context, device, properties, &err))
    return *this;
}

// CommandQueuePointer::CommandQueuePointer(Application &app, cl_command_queue_properties properties) : Pointer<cl::CommandQueue>(app, app.getContext(), app.getDevice(), properties, &app.getErr()){}