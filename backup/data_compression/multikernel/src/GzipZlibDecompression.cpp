#include "GzipZlibDecompression.hpp"
#include <sstream>
#include "DataCompression.hpp"

void GzipZlibDecompressionKernelExecutor::process(){
	std::thread chunkWriter([this]{
		CommandQueuePointer chunkWriterStream(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
		std::stringstream chunkWriterKernelName;
		chunkWriterKernelName<<"xilMM2S:{xilMM2S_"<<idx<<"}";
		KernelPointer chunkWriterKernel(Application::getProgram<Lib::GZIP_ZLIB_DECOMPRESSION>(), chunkWriterKernelName.str());

		// host allocated aligned memory
		std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(GzipZlibDecompressionWorkshop::CHUNK_SIZE_IN_BYTE);
		BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, GzipZlibDecompressionWorkshop::CHUNK_SIZE_IN_BYTE, inputBufferHost.data());
		
		chunkWriterKernel->setArg(0, *inputBuffer);

		bool last=0;
		do{
			GzipZlibDecompressionItem item=inputStream.pop();
			last=item.last;
			uint32_t chunkSize=item.size;
			memcpy(inputBufferHost.data(), item.payload, chunkSize);
			delete[] item.payload;
			std::cout<<"inner reads a "<<chunkSize<<" Bytes block"<<std::endl;

			chunkWriterStream->enqueueMigrateMemObjects({*inputBuffer}, 0);
			chunkWriterStream->finish();

			chunkWriterKernel->setArg(1, chunkSize);
			chunkWriterKernel->setArg(2, (uint32_t)last);

			chunkWriterStream->enqueueTask(*chunkWriterKernel);
			chunkWriterStream->finish();
		}while(!last);
	});

	std::thread chunkReader([this]{
		CommandQueuePointer chunkReaderStream(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
		std::stringstream chunkReaderKernelName;
		chunkReaderKernelName<<"xilS2MM:{xilS2MM_"<<idx<<"}";
		KernelPointer chunkReaderKernel(Application::getProgram<Lib::GZIP_ZLIB_DECOMPRESSION>(), chunkReaderKernelName.str());

		std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(GzipZlibDecompressionWorkshop::CHUNK_SIZE_IN_BYTE);
		std::vector<uint32_t, aligned_allocator<uint32_t>> outputSizeBufferHost(1);
		std::vector<uint32_t, aligned_allocator<uint32_t>> statusBufferHost(1);
		statusBufferHost[0]=0;

		BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, GzipZlibDecompressionWorkshop::CHUNK_SIZE_IN_BYTE, outputBufferHost.data());
		BufferPointer outputSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), outputSizeBufferHost.data());
		BufferPointer statusBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), statusBufferHost.data());

		chunkReaderKernel->setArg(0, *outputBuffer);
		chunkReaderKernel->setArg(1, *outputSizeBuffer);
		chunkReaderKernel->setArg(2, *statusBuffer);
		chunkReaderKernel->setArg(3, (uint32_t)outputBufferHost.size());

		do{
			chunkReaderStream->enqueueMigrateMemObjects({*statusBuffer}, 0);
			chunkReaderStream->finish();

			chunkReaderStream->enqueueTask(*chunkReaderKernel);
			chunkReaderStream->finish();

			chunkReaderStream->enqueueMigrateMemObjects({*outputSizeBuffer, *statusBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
			chunkReaderStream->finish();

			chunkReaderStream->enqueueMigrateMemObjects({*outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
			chunkReaderStream->finish();

			outputStream.push(outputBufferHost.data(), outputSizeBufferHost[0], statusBufferHost[0]);
			std::cout<<"inner writes a "<<outputSizeBufferHost[0]<<" Bytes block"<<std::endl;
		}while(!statusBufferHost[0]);
	});

	chunkWriter.join();
	chunkReader.join();

	std::cout<<"End of kernel executor "<<idx<<std::endl;
}

GzipZlibDecompressionKernelExecutor::GzipZlibDecompressionKernelExecutor(
	int idx,
	Stream<GzipZlibDecompressionItem> &inputStream, 
	ByteStream &outputStream,  GzipZlibDecompressionWorkshop &workshop) 
	: KernelExecutor<GzipZlibDecompressionItem>(idx, inputStream, outputStream), workshop(workshop){}

void GzipZlibDecompressionStructureAnalyzer::process(){
	bool last;
	uint64_t idx=0;

	// compress the input block by block
	do{
		uint8_t *payload=new uint8_t[GzipZlibDecompressionWorkshop::CHUNK_SIZE_IN_BYTE];
		uint32_t chunkSize=inputStream.pop(payload, GzipZlibDecompressionWorkshop::CHUNK_SIZE_IN_BYTE, last);
		std::cout<<"inner reads a "<<chunkSize<<" Bytes block"<<std::endl;

		GzipZlibDecompressionItem item;
		item.idx=idx++;
		item.size=chunkSize;
		item.payload=payload;
		item.last=last;
		outputStream.push(item);
	}while(!last);

	std::cout<<"End of structure analyzer"<<std::endl;
}

GzipZlibDecompressionStructureAnalyzer::GzipZlibDecompressionStructureAnalyzer(
		ByteStream &inputStream,
		Stream<GzipZlibDecompressionItem> &outputStream) : 
		StructureAnalyzer<GzipZlibDecompressionItem>(inputStream, outputStream){}

GzipZlibDecompressionWorkshop::GzipZlibDecompressionWorkshop() : 
	Workshop<GzipZlibDecompressionItem>("GzipZlibInputStream", 4, "GzipZlibBridgeStream", 2, "GzipZlibOutputStream", 4), 
	kernelExecutor(1, bridgeStream, outputStream, *this),
	structureAnalyzer(inputStream, bridgeStream),
	structureAnalyzerThread(&GzipZlibDecompressionStructureAnalyzer::process, &structureAnalyzer),
	kernelExecutorThread(&GzipZlibDecompressionKernelExecutor::process, &kernelExecutor){
}

void GzipZlibDecompressionWorkshop::wait(){
	structureAnalyzerThread.join();

	std::cout<<"structureAnalyzerThread is joined"<<std::endl;

	kernelExecutorThread.join();

	std::cout<<"kernelExecutorThread is joined"<<std::endl;
}

ByteStream& GzipZlibDecompressionWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& GzipZlibDecompressionWorkshop::getOutputStream(){
	return outputStream;
}