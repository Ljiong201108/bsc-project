#pragma once

#include "Util.hpp"
#include "Pointer.hpp"

class CommandQueuePointer : public Pointer<cl::CommandQueue>{
public:
    CommandQueuePointer();
    CommandQueuePointer &create(const cl::Context& context, const cl::Device& device, cl_command_queue_properties properties=0);
};