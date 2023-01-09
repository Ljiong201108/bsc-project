#pragma once

#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <utility>

#include "Util.hpp"
#include "ThreadSafeQueue.hpp"
#include "Workshop.hpp"
#include "HostCommon.hpp"

template<typename T>
class KernelExecutor{
protected:
    int idx;

    ThreadSafeQueue<T> &inputQueue;
    GeneralQueue &outputQueue;

public:
    KernelExecutor(
        int idx,
        ThreadSafeQueue<T> &inputQueue, 
        GeneralQueue &outputQueue);
    virtual void process()=0;
};

template<typename T>
KernelExecutor<T>::KernelExecutor(
    int idx,
    ThreadSafeQueue<T> &inputQueue, 
    GeneralQueue &outputQueue) 
    : idx(idx), inputQueue(inputQueue), outputQueue(outputQueue){
}

// void KernelExecutor::process(){

//     std::vector<cl::UserEvent> inputEvents;
//     std::vector<cl::UserEvent> migrationH2DEvents;
//     std::vector<cl::UserEvent> kernelExeutionEvents;
//     std::vector<cl::UserEvent> migrationD2HEvents;
//     std::vector<cl::UserEvent> outputEvents;

//     std::vector<std::thread> inputThreads, outputThreads;

//     uint64_t cur=0;
//     cl_int err;
//     bool last;

//     while(true){
//         auto [idx, config, buffer]=inputQueue.pop(last);

//         if(idx==-1) break;

//         // OCL_CHECK(err, inputEvents.emplace_back(getContext(), err))
//         // OCL_CHECK(err, migrationH2DEvents.emplace_back(getContext(), err))
//         // OCL_CHECK(err, kernelExeutionEvents.emplace_back(getContext(), err))
//         // OCL_CHECK(err, migrationD2HEvents.emplace_back(getContext(), err))
//         // OCL_CHECK(err, outputEvents.emplace_back(getContext(), err))

//         // inputThreads.emplace_back(&SingleKernelExecutor::executeInput, this, cur, inputEvents, migrationH2DEvents, kernelExeutionEvents, migrationD2HEvents, outputEvents);
//         // executeMigrationH2D(cur, inputEvents, migrationH2DEvents, kernelExeutionEvents, migrationD2HEvents, outputEvents);
//         // executeKernel(cur, inputEvents, migrationH2DEvents, kernelExeutionEvents, migrationD2HEvents, outputEvents);
//         // executeMigrationD2H(cur, inputEvents, migrationH2DEvents, kernelExeutionEvents, migrationD2HEvents, outputEvents);
//         // outputThreads.emplace_back(&SingleKernelExecutor::executeOutput, this, cur, inputEvents, migrationH2DEvents, kernelExeutionEvents, migrationD2HEvents, outputEvents);
//         cur++;
//     }

//     for(auto &t : inputThreads) t.join();
//     for(auto &t : outputThreads) t.join();
//     for(auto &e : inputEvents) e.wait();
//     for(auto &e : migrationH2DEvents) e.wait();
//     for(auto &e : kernelExeutionEvents) e.wait();
//     for(auto &e : migrationD2HEvents) e.wait();
//     for(auto &e : outputEvents) e.wait();
// };