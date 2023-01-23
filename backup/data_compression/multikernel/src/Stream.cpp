#include "Stream.hpp"

ByteItem::ByteItem(uint32_t size, uint8_t *payload, bool last) : size(size), used(0), payload(payload), last(last){}

uint32_t ByteItem::available(){
	return size-used;
}

void ByteItem::use(uint8_t *dest, uint32_t size){
	assert(size<=available() && "size should smaller than available");

	// std::cout<<"using item for "<<size<<" Bytes: size="<<this->size<<" used="<<this->used<<" last="<<last<<std::endl;
	std::memcpy(dest, payload+used, size);
	used+=size;
}

bool ByteItem::empty(){
	return size==used;
}

ByteStream::ByteStream(std::string name) : Stream<ByteItem>(name){}

ByteStream::ByteStream(std::string name, uint64_t maxSize) : Stream<ByteItem>(name, maxSize){}

void ByteStream::push(void *SRC, uint32_t size, bool last){
	uint8_t *src=(uint8_t*)SRC;
	uint8_t *payload=new uint8_t[size];
	memcpy(payload, src, size);
	std::unique_lock<std::mutex> lk(mut);
	cv.wait(lk, [this] { return queue.size()<maxSize; });
	queue.emplace(size, payload, last);
	cv.notify_all();
}

uint32_t ByteStream::pop(void *DEST, uint32_t size, bool &last){
	uint8_t *dest=(uint8_t*)DEST;
	std::unique_lock<std::mutex> lk(mut);
	uint32_t popped=0;
	while(size){
		cv.wait(lk, [this] { return queue.size()>0; });
		ByteItem &item=queue.front();
		uint32_t canUse=std::min(size, item.available());
		item.use(dest, canUse);
		dest+=canUse;
		popped+=canUse;
		size-=canUse;
		if(item.empty()){
			delete[] item.payload;
			if(item.last){
				last=true;
				queue.pop();
				return popped;
			}
			queue.pop();
			cv.notify_all();
		}
	}

	last=false;
	return popped;
}