#pragma once

#include <vector>

#include "Stream.hpp"

class Workshop{
protected:
	ByteStream inputStream, outputStream;

public:
	Workshop(
		std::string nameInputStream, uint64_t depthInputStream, 
		std::string nameOutputStream, uint64_t depthOutputStream);

	virtual ~Workshop()=default;
	virtual ByteStream& getInputStream();
	virtual ByteStream& getOutputStream();
};
