#pragma once

#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <utility>

#include "Util.hpp"
#include "Stream.hpp"
#include "Workshop.hpp"
#include "HostCommon.hpp"

template<typename T>
class KernelExecutor{
protected:
    int idx;

    Stream<T> &inputStream;
    ByteStream &outputStream;

public:
    KernelExecutor(
        int idx,
        Stream<T> &inputStream, 
        ByteStream &outputStream);
    virtual void process()=0;
};

template<typename T>
KernelExecutor<T>::KernelExecutor(
    int idx,
    Stream<T> &inputStream, 
    ByteStream &outputStream) 
    : idx(idx), inputStream(inputStream), outputStream(outputStream){
}
