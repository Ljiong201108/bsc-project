#pragma once

#include <stdint.h>
#include <chrono>
#include <memory>
#include <algorithm>

/**
 * also not thread safe, can be safe later
*/
template<typename T, uint32_t N>
class Pool{
protected:
    std::unique_ptr<T[]> m_poolItems;

public:
    Pool();

    template<uint32_t I>
    T &get();
};

template<typename T, uint32_t N>
Pool<T, N>::Pool() : m_poolItems{std::make_unique<T[]>(N)}{
    for(uint32_t i=0;i<N;i++) m_poolItems[i]=T();
}

template<typename T, uint32_t N>
template<uint32_t I>
T &Pool<T, N>::get(){
    static_assert(0<=I && I<N, "Index out of bound!");
    return m_poolItems[I];
}
