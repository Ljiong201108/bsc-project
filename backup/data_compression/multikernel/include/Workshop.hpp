#pragma once

#include <vector>

#include "KernelExecutor.hpp"
#include "Stream.hpp"

template<typename T>
class Workshop{
protected:
    uint64_t outputIdx;
    std::mutex mtx;
    std::condition_variable cv;
    
    Stream<T> bridgeStream;
    ByteStream inputStream, outputStream;

public:
    Workshop(
		std::string nameInputStream, uint64_t depthInputStream, 
		std::string nameBridgeStream, uint64_t depthBridgeStream, 
		std::string nameOutputStream, uint64_t depthOutputStream);

    virtual ~Workshop()=default;
    virtual ByteStream& getInputStream();
    virtual ByteStream& getOutputStream();
};

template<typename T>
Workshop<T>::Workshop(
		std::string nameInputStream, uint64_t depthInputStream, 
		std::string nameBridgeStream, uint64_t depthBridgeStream, 
		std::string nameOutputStream, uint64_t depthOutputStream) : 
	outputIdx(0), 
	bridgeStream(nameInputStream, depthBridgeStream), 
	inputStream(nameBridgeStream, depthInputStream), 
	outputStream(nameOutputStream, depthOutputStream){}

template<typename T>
ByteStream& Workshop<T>::getInputStream(){
    return inputStream;
}

template<typename T>
ByteStream& Workshop<T>::getOutputStream(){
    return outputStream;
}