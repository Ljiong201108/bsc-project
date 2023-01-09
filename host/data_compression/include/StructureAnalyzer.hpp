#pragma once

#include "ThreadSafeQueue.hpp"

template<typename T>
class StructureAnalyzer{
protected:
    GeneralQueue &inputQueue;
    ThreadSafeQueue<T> &outputQueue;

public:
    StructureAnalyzer(
        GeneralQueue &inputQueue,
        ThreadSafeQueue<T> &outputQueue);

    virtual void process()=0;
};

template<typename T>
StructureAnalyzer<T>::StructureAnalyzer(
    GeneralQueue &inputQueue,
    ThreadSafeQueue<T> &outputQueue) 
    : inputQueue(inputQueue), outputQueue(outputQueue){}