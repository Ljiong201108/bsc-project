#pragma once

#include <queue>
#include <utility>
#include <thread>
#include <condition_variable>
#include <stdint.h>
#include <iostream>
#include <assert.h>
#include <cstring>

#define DEFAULT_MAX_SIZE (128)

template<typename T>
class ThreadSafeQueue {
protected:
    mutable std::mutex mut;
    std::queue<T> queue;
    std::condition_variable cv;
    std::string name;
	uint64_t maxSize;

public:
    ThreadSafeQueue(std::string name);
    ThreadSafeQueue(std::string name, uint64_t maxSize);
    ~ThreadSafeQueue();
    virtual void push(T val);
    virtual T pop();
};

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(std::string name, uint64_t maxSize) : name(name), maxSize(maxSize){}

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(std::string name) : ThreadSafeQueue(name, DEFAULT_MAX_SIZE){}

template<typename T>
ThreadSafeQueue<T>::~ThreadSafeQueue(){
    if(!queue.empty()) std::cout<<"Warning: FIFO "<<name<<" is not empty!"<<std::endl;
}

template<typename T>
void ThreadSafeQueue<T>::push(T val) {
    std::unique_lock<std::mutex> lk(mut);
    cv.wait(lk, [this] { return queue.size()<maxSize; });
    queue.push(val);
    cv.notify_all();
}

template<typename T>
T ThreadSafeQueue<T>::pop() {
    std::unique_lock<std::mutex> lk(mut);
    cv.wait(lk, [this] { return queue.size()>0; });
    T val=queue.front();
    queue.pop();
    cv.notify_all();
    return val;
}

struct GeneralItem{
	uint32_t size;
	uint32_t used;
	uint8_t *payload;
	bool last;

	GeneralItem(uint32_t size, uint8_t *payload, bool last);
	uint32_t available();
	void use(uint8_t *dest, uint32_t size);
	bool empty();
};

class GeneralQueue : ThreadSafeQueue<GeneralItem>{
public:
	GeneralQueue(std::string name);
	GeneralQueue(std::string name, uint64_t maxSize);
	void push(void *src, uint32_t size, bool last);
	uint32_t pop(void *dest, uint32_t size, bool &last);
};