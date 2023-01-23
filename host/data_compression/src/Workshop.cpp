#include "Workshop.hpp"

Workshop::Workshop(
		std::string nameInputStream, uint64_t depthInputStream, 
		std::string nameOutputStream, uint64_t depthOutputStream) : 
	inputStream(nameInputStream, depthInputStream), 
	outputStream(nameOutputStream, depthOutputStream){}

ByteStream& Workshop::getInputStream(){
	return inputStream;
}

ByteStream& Workshop::getOutputStream(){
	return outputStream;
}