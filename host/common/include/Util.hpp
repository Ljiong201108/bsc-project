#pragma once

#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdlib.h>
// #include <chorno>

#define CL_HPP_CL_1_2_DEFAULT_BUILD                                    
#define CL_HPP_TARGET_OPENCL_VERSION 120                               
#define CL_HPP_MINIMUM_OPENCL_VERSION 120                            
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS      
#include <CL/cl2.hpp>

#define ERROR_CASE(err) \
    case err:           \
        return #err;    \
        break

inline static const char* error_string(cl_int error_code) {
    switch (error_code) {
        ERROR_CASE(CL_SUCCESS);
        ERROR_CASE(CL_DEVICE_NOT_FOUND);
        ERROR_CASE(CL_DEVICE_NOT_AVAILABLE);
        ERROR_CASE(CL_COMPILER_NOT_AVAILABLE);
        ERROR_CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE);
        ERROR_CASE(CL_OUT_OF_RESOURCES);
        ERROR_CASE(CL_OUT_OF_HOST_MEMORY);
        ERROR_CASE(CL_PROFILING_INFO_NOT_AVAILABLE);
        ERROR_CASE(CL_MEM_COPY_OVERLAP);
        ERROR_CASE(CL_IMAGE_FORMAT_MISMATCH);
        ERROR_CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED);
        ERROR_CASE(CL_BUILD_PROGRAM_FAILURE);
        ERROR_CASE(CL_MAP_FAILURE);
        ERROR_CASE(CL_INVALID_VALUE);
        ERROR_CASE(CL_INVALID_DEVICE_TYPE);
        ERROR_CASE(CL_INVALID_PLATFORM);
        ERROR_CASE(CL_INVALID_DEVICE);
        ERROR_CASE(CL_INVALID_CONTEXT);
        ERROR_CASE(CL_INVALID_QUEUE_PROPERTIES);
        ERROR_CASE(CL_INVALID_COMMAND_QUEUE);
        ERROR_CASE(CL_INVALID_HOST_PTR);
        ERROR_CASE(CL_INVALID_MEM_OBJECT);
        ERROR_CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
        ERROR_CASE(CL_INVALID_IMAGE_SIZE);
        ERROR_CASE(CL_INVALID_SAMPLER);
        ERROR_CASE(CL_INVALID_BINARY);
        ERROR_CASE(CL_INVALID_BUILD_OPTIONS);
        ERROR_CASE(CL_INVALID_PROGRAM);
        ERROR_CASE(CL_INVALID_PROGRAM_EXECUTABLE);
        ERROR_CASE(CL_INVALID_KERNEL_NAME);
        ERROR_CASE(CL_INVALID_KERNEL_DEFINITION);
        ERROR_CASE(CL_INVALID_KERNEL);
        ERROR_CASE(CL_INVALID_ARG_INDEX);
        ERROR_CASE(CL_INVALID_ARG_VALUE);
        ERROR_CASE(CL_INVALID_ARG_SIZE);
        ERROR_CASE(CL_INVALID_KERNEL_ARGS);
        ERROR_CASE(CL_INVALID_WORK_DIMENSION);
        ERROR_CASE(CL_INVALID_WORK_GROUP_SIZE);
        ERROR_CASE(CL_INVALID_WORK_ITEM_SIZE);
        ERROR_CASE(CL_INVALID_GLOBAL_OFFSET);
        ERROR_CASE(CL_INVALID_EVENT_WAIT_LIST);
        ERROR_CASE(CL_INVALID_EVENT);
        ERROR_CASE(CL_INVALID_OPERATION);
        ERROR_CASE(CL_INVALID_GL_OBJECT);
        ERROR_CASE(CL_INVALID_BUFFER_SIZE);
        ERROR_CASE(CL_INVALID_MIP_LEVEL);
        ERROR_CASE(CL_INVALID_GLOBAL_WORK_SIZE);
        ERROR_CASE(CL_COMPILE_PROGRAM_FAILURE);
        ERROR_CASE(CL_LINKER_NOT_AVAILABLE);
        ERROR_CASE(CL_LINK_PROGRAM_FAILURE);
        ERROR_CASE(CL_DEVICE_PARTITION_FAILED);
        ERROR_CASE(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
        ERROR_CASE(CL_INVALID_PROPERTY);
        ERROR_CASE(CL_INVALID_IMAGE_DESCRIPTOR);
        ERROR_CASE(CL_INVALID_COMPILER_OPTIONS);
        ERROR_CASE(CL_INVALID_LINKER_OPTIONS);
        ERROR_CASE(CL_INVALID_DEVICE_PARTITION_COUNT);
        default:
            std::cerr << "Unknown OpenCL Error" << error_code << std::endl;
            break;
    }
    return nullptr;
}

#define OCL_CHECK(error, call)                                          \
    call;                                                               \
    if (error != CL_SUCCESS) {                                          \
        std::cout << __FILE__ << ":" << __LINE__ << " OPENCL API --> "; \
        std::cout << #call;                                             \
        std::cout << ", RESULT: -->  ";                                 \
        std::cout << error_string(error) << std::endl;                  \
        exit(EXIT_FAILURE);                                             \
    }     

// When creating a buffer with user pointer (CL_MEM_USE_HOST_PTR), under the hood
// User ptr is used if and only if it is properly aligned (page aligned). When not
// aligned, runtime has no choice but to create its own host side buffer that backs
// user ptr. This in turn implies that all operations that move data to and from
// device incur an extra memcpy to move data to/from runtime's own host buffer
// from/to user pointer. So it is recommended to use this allocator if user wish to
// Create Buffer/Memory Object with CL_MEM_USE_HOST_PTR to align user buffer to the
// page boundary. It will ensure that user buffer will be used when user create
// Buffer/Mem Object with CL_MEM_USE_HOST_PTR.
template <typename T>
struct aligned_allocator {
    using value_type = T;
    T* allocate(std::size_t num) {
        void* ptr = nullptr;
        if (posix_memalign(&ptr, 4096, num * sizeof(T))) throw std::bad_alloc();
        return reinterpret_cast<T*>(ptr);
    }
    void deallocate(T* p, std::size_t num) { free(p); }
};

inline std::unique_ptr<uint64_t[]> arrayAlignedPadding(uint8_t *array, uint64_t sizeInByte, uint64_t alignedSizeInByte, uint64_t &newSizeInByte){
    newSizeInByte=(sizeInByte/alignedSizeInByte+((sizeInByte%alignedSizeInByte)>0))*alignedSizeInByte;
    std::cout<<"size in Byte is "<<sizeInByte<<std::endl;
    std::cout<<"new size in Byte is "<<newSizeInByte<<std::endl;
    std::unique_ptr<uint64_t[]> newArray=std::make_unique<uint64_t[]>(newSizeInByte/sizeof(uint64_t));
    std::memcpy(newArray.get(), array, sizeInByte);
    std::fill(newArray.get()+sizeInByte, newArray.get()+newSizeInByte, (uint8_t)(newSizeInByte-sizeInByte));
    return newArray;
}

inline std::unique_ptr<uint64_t[]> arrayAlignedPadding(uint8_t *array, uint32_t sizeInByte, uint32_t alignedSizeInByte, uint32_t &newSizeInByte){
    newSizeInByte=(sizeInByte/alignedSizeInByte+((sizeInByte%alignedSizeInByte)>0))*alignedSizeInByte;
    std::cout<<"size in Byte is "<<sizeInByte<<std::endl;
    std::cout<<"new size in Byte is "<<newSizeInByte<<std::endl;
    std::unique_ptr<uint64_t[]> newArray=std::make_unique<uint64_t[]>(newSizeInByte/sizeof(uint64_t));
    std::memcpy(newArray.get(), array, sizeInByte);
    std::fill(newArray.get()+sizeInByte, newArray.get()+newSizeInByte, (uint8_t)(newSizeInByte-sizeInByte));
    return newArray;
}

inline std::string xclbinPath(){
    char *pathvar=getenv("XCLBIN_PATH");
    return std::string(pathvar);
}