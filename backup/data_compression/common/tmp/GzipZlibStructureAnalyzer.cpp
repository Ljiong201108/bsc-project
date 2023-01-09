#include "GzipZlibWorkshop.hpp"

void GzipZlibCompressionStructureAnalyzer::process(){
	bool last;
	uint64_t idx=0;

	// compress the input block by block
	do{
		uint8_t *payload=new uint8_t[GzipZlibCompressionWorkshop::CHUNK_SIZE_IN_BYTE];
		uint32_t chunkSize=inputQueue.pop(payload, GzipZlibCompressionWorkshop::CHUNK_SIZE_IN_BYTE, last);
		std::cout<<"inner reads a "<<chunkSize<<" Bytes block"<<std::endl;

		GzipZlibCompressionItem item;
		item.idx=idx++;
		item.size=chunkSize;
		item.payload=payload;
		outputQueue.push(item);
	}while(!last);

	std::cout<<"End of structure analyzer"<<std::endl;
}

GzipZlibCompressionStructureAnalyzer::GzipZlibCompressionStructureAnalyzer(
		GeneralQueue &inputQueue,
		ThreadSafeQueue<GzipZlibCompressionItem> &outputQueue) : 
		StructureAnalyzer<GzipZlibCompressionItem>(inputQueue, outputQueue){}