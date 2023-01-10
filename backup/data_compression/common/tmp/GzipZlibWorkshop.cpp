#include "GzipZlibWorkshop.hpp"

GzipZlibCompressionWorkshop::GzipZlibCompressionWorkshop(bool isZlib) : 
	Workshop<GzipZlibCompressionItem>("GzipZlibInputQueue", 4, "GzipZlibBridgeQueue", 2, "GzipZlibOutputQueue", 4), 
	kernelExecutor(1, isZlib, bridgeQueue, outputQueue, *this),
	structureAnalyzer(inputQueue, bridgeQueue),
	structureAnalyzerThread(&GzipZlibCompressionStructureAnalyzer::process, &structureAnalyzer),
	kernelExecutorThread(&GzipZlibCompressionKernelExecutor::process, &kernelExecutor){
}

void GzipZlibCompressionWorkshop::wait(){
	structureAnalyzerThread.join();

	std::cout<<"structureAnalyzerThread is joined"<<std::endl;

	GzipZlibCompressionItem item;
	item.idx=(uint64_t)-1;
	item.size=0;
	item.payload=nullptr;
	bridgeQueue.push(item);

	kernelExecutorThread.join();

	std::cout<<"kernelExecutorThread is joined"<<std::endl;

	// Add last block header
	long int last_block = 0xffff000001;
	outputQueue.push(&last_block, 5, false);

	uint8_t tmp=0;
	outputQueue.push(&tmp, 1, true);

	std::cout<<"End of workshop wait"<<std::endl;
}

ByteQueue& GzipZlibCompressionWorkshop::getInputQueue(){
	return inputQueue;
}

ByteQueue& GzipZlibCompressionWorkshop::getOutputQueue(){
	return outputQueue;
}