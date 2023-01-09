#pragma once

#include <queue>
#include <utility>
#include <thread>
#include <condition_variable>
#include <stdint.h>
#include <iostream>
#include <assert.h>

#define DEFAULT_MAX_SIZE (128)

template<typename T>
class ThreadSafeFIFO {
private:
    mutable std::mutex mut;
    std::queue<std::pair<T, bool>> queue;
    std::condition_variable cv;
    uint64_t maxSize;

public:
    ThreadSafeFIFO();
    ThreadSafeFIFO(uint64_t maxSize);
    ~ThreadSafeFIFO();
    void push(T val, bool last);
    std::pair<T, bool> pop();
    T pop(bool &last);
    void push(void *val, uint32_t num, bool last);
    uint32_t pop(void *val, uint32_t num, bool &last);
};

template<typename T>
ThreadSafeFIFO<T>::ThreadSafeFIFO(uint64_t maxSize) : maxSize(maxSize){}

template<typename T>
ThreadSafeFIFO<T>::ThreadSafeFIFO() : ThreadSafeFIFO(DEFAULT_MAX_SIZE){}

template<typename T>
ThreadSafeFIFO<T>::~ThreadSafeFIFO(){
    if(!queue.empty()) std::cout<<"Warning: FIFO is not empty!"<<std::endl;
}

template<typename T>
void ThreadSafeFIFO<T>::push(T val, bool last) {
    std::unique_lock<std::mutex> lk(mut);
    cv.wait(lk, [this] { return queue.size()<maxSize; });
    queue.emplace(val, last);
    cv.notify_all();
}

template<typename T>
void ThreadSafeFIFO<T>::push(void *val, uint32_t num, bool last) {
    assert(num!=0 && "ThreadSafeFIFO cannot push empty!");
    T *valT=(T*)val;
    for(uint32_t i=0;i<num-1;i++) push(valT[i], false);
    push(valT[num-1], last);
}

template<typename T>
std::pair<T, bool> ThreadSafeFIFO<T>::pop() {
    std::unique_lock<std::mutex> lk(mut);
    cv.wait(lk, [this] { return queue.size()>0; });
    std::pair<T, bool> val=queue.front();
    queue.pop();
    cv.notify_all();
    return val;
}

template<typename T>
T ThreadSafeFIFO<T>::pop(bool &last) {
    std::unique_lock<std::mutex> lk(mut);
    cv.wait(lk, [this] { return queue.size()>0; });
    std::pair<T, bool> val=queue.front();
    queue.pop();
    last=val.second;
    cv.notify_all();
    return val.first;
}

template<typename T>
uint32_t ThreadSafeFIFO<T>::pop(void *val, uint32_t num, bool &last) {
    assert(num!=0 && "ThreadSafeFIFO cannot pop empty!");
    T *valT=(T*)val;
    uint32_t realNum=0;
    do{
        auto [curVal, curLast]=pop();
        last=curLast;
        valT[realNum++]=curVal;
    }while(!last && realNum<num);
    return realNum;
}