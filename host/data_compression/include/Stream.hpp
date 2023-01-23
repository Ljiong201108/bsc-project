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
class Stream {
protected:
    mutable std::mutex mut;
    std::queue<T> queue;
    std::condition_variable cv;
    std::string name;
	uint64_t maxSize;

public:
    Stream(std::string name);
    Stream(std::string name, uint64_t maxSize);
    ~Stream();
    virtual void push(T val);
    virtual T pop();
};

template<typename T>
Stream<T>::Stream(std::string name, uint64_t maxSize) : name(name), maxSize(maxSize){}

template<typename T>
Stream<T>::Stream(std::string name) : Stream(name, DEFAULT_MAX_SIZE){}

template<typename T>
Stream<T>::~Stream(){
    if(!queue.empty()) std::cout<<"Warning: Stream "<<name<<" is not empty!"<<std::endl;
}

template<typename T>
void Stream<T>::push(T val) {
    std::unique_lock<std::mutex> lk(mut);
    cv.wait(lk, [this] { return queue.size()<maxSize; });
    queue.push(val);
    cv.notify_all();
}

template<typename T>
T Stream<T>::pop() {
    std::unique_lock<std::mutex> lk(mut);
    cv.wait(lk, [this] { return queue.size()>0; });
    T val=queue.front();
    queue.pop();
    cv.notify_all();
    return val;
}

struct ByteItem{
	uint32_t size;
	uint32_t used;
	uint8_t *payload;
	bool last;

	ByteItem(uint32_t size, uint8_t *payload, bool last);
	uint32_t available();
	void use(uint8_t *dest, uint32_t size);
	bool empty();
};

class ByteStream : Stream<ByteItem>{
public:
	ByteStream(std::string name);
	ByteStream(std::string name, uint64_t maxSize);
	void push(void *src, uint32_t size, bool last);
	uint32_t pop(void *dest, uint32_t size, bool &last);
	uint8_t pop(bool &last);
};