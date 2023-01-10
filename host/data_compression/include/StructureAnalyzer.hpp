#pragma once

#include "Stream.hpp"

template<typename T>
class StructureAnalyzer{
protected:
    ByteStream &inputStream;
    Stream<T> &outputStream;

public:
    StructureAnalyzer(
        ByteStream &inputStream,
        Stream<T> &outputStream);

    virtual void process()=0;
};

template<typename T>
StructureAnalyzer<T>::StructureAnalyzer(
    ByteStream &inputStream,
    Stream<T> &outputStream) 
    : inputStream(inputStream), outputStream(outputStream){}