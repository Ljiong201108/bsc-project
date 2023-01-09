#pragma once

#include <vector>

#include "KernelExecutor.hpp"
#include "ThreadSafeQueue.hpp"

template<typename T>
class Workshop{
protected:
    uint64_t outputIdx;
    std::mutex mtx;
    std::condition_variable cv;
    
    ThreadSafeQueue<T> bridgeQueue;
    GeneralQueue inputQueue, outputQueue;

public:
    Workshop(
		std::string nameInputQueue, uint64_t depthInputQueue, 
		std::string nameBridgeQueue, uint64_t depthBridgeQueue, 
		std::string nameOutputQueue, uint64_t depthOutputQueue);

    virtual ~Workshop()=default;
    virtual GeneralQueue& getInputQueue();
    virtual GeneralQueue& getOutputQueue();
};

template<typename T>
Workshop<T>::Workshop(
		std::string nameInputQueue, uint64_t depthInputQueue, 
		std::string nameBridgeQueue, uint64_t depthBridgeQueue, 
		std::string nameOutputQueue, uint64_t depthOutputQueue) : 
	outputIdx(0), 
	bridgeQueue(nameInputQueue, depthBridgeQueue), 
	inputQueue(nameBridgeQueue, depthInputQueue), 
	outputQueue(nameOutputQueue, depthOutputQueue){}

template<typename T>
GeneralQueue& Workshop<T>::getInputQueue(){
    return inputQueue;
}

template<typename T>
GeneralQueue& Workshop<T>::getOutputQueue(){
    return outputQueue;
}