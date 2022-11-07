#pragma once

#include <memory>
#include <cstring>
#include <chrono>

template <typename T>
class Pointer{
protected:
    std::unique_ptr<T> m_content;

    Pointer();

public:
    template<typename... ARGS>
    void build(const ARGS&... args);

    T &operator*();

    T *operator->();
};

template <typename T>
Pointer<T>::Pointer() : m_content{nullptr}{}

template <typename T>
template <typename... ARGS>
void Pointer<T>::build(const ARGS&... args){
    m_content.reset(nullptr);
    m_content=std::make_unique<T>(args...);
}

template <typename T>
T& Pointer<T>::operator*(){
    return m_content.operator*();
}

template <typename T>
T *Pointer<T>::operator->(){
    return m_content.operator->();
}