#include "GzipZlibDecompress.hpp"

void GzipZlibDecompressWorkshop::process(){
	std::thread chunkWriter([this]{
		CommandQueuePointer chunkWriterQueue(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
		KernelPointer chunkWriterKernel(Application::getProgram<Lib::GZIP_ZLIB_DECOMPRESSION>(), "xilMM2S:{xilMM2S_1}");

		// host allocated aligned memory
		std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(CHUNK_SIZE_IN_BYTE);
		BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, CHUNK_SIZE_IN_BYTE, inputBufferHost.data());
		
		chunkWriterKernel->setArg(0, *inputBuffer);

		bool last=0;
		do{
			uint32_t chunkSize=inputStream.pop(inputBufferHost.data(), CHUNK_SIZE_IN_BYTE, last);
			std::cout<<"inner reads a "<<chunkSize<<" Bytes block"<<std::endl;

			chunkWriterQueue->enqueueMigrateMemObjects({*inputBuffer}, 0);
			chunkWriterQueue->finish();

			chunkWriterKernel->setArg(1, chunkSize);
			chunkWriterKernel->setArg(2, (uint32_t)last);

			chunkWriterQueue->enqueueTask(*chunkWriterKernel);
			chunkWriterQueue->finish();
		}while(!last);
	});

	std::thread chunkReader([this]{
		CommandQueuePointer chunkReaderQueue(Application::getContext(), Application::getDevice(), CL_QUEUE_PROFILING_ENABLE);
		KernelPointer chunkReaderKernel(Application::getProgram<Lib::GZIP_ZLIB_DECOMPRESSION>(), "xilS2MM:{xilS2MM_1}");

		std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(CHUNK_SIZE_IN_BYTE);
		std::vector<uint32_t, aligned_allocator<uint32_t>> outputSizeBufferHost(1);
		std::vector<uint32_t, aligned_allocator<uint32_t>> statusBufferHost(1);
		statusBufferHost[0]=0;

		BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, CHUNK_SIZE_IN_BYTE, outputBufferHost.data());
		BufferPointer outputSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), outputSizeBufferHost.data());
		BufferPointer statusBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), statusBufferHost.data());

		chunkReaderKernel->setArg(0, *outputBuffer);
		chunkReaderKernel->setArg(1, *outputSizeBuffer);
		chunkReaderKernel->setArg(2, *statusBuffer);
		chunkReaderKernel->setArg(3, (uint32_t)outputBufferHost.size());

		do{
			chunkReaderQueue->enqueueMigrateMemObjects({*statusBuffer}, 0);
			chunkReaderQueue->finish();

			chunkReaderQueue->enqueueTask(*chunkReaderKernel);
			chunkReaderQueue->finish();

			chunkReaderQueue->enqueueMigrateMemObjects({*outputSizeBuffer, *statusBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
			chunkReaderQueue->finish();

			chunkReaderQueue->enqueueMigrateMemObjects({*outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
			chunkReaderQueue->finish();

			std::cout<<"inner writes a "<<outputSizeBufferHost[0]<<" Bytes block"<<std::endl;
			outputStream.push(outputBufferHost.data(), outputSizeBufferHost[0], statusBufferHost[0]);
		}while(!statusBufferHost[0]);
	});

	chunkWriter.join();
	chunkReader.join();
}

GzipZlibDecompressWorkshop::GzipZlibDecompressWorkshop() : 
	Workshop("GzipZlibInputStream", 8, "GzipZlibOutputStream", 8),
	processThread(&GzipZlibDecompressWorkshop::process, this){}

void GzipZlibDecompressWorkshop::wait(){
	processThread.join();
}

ByteStream& GzipZlibDecompressWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& GzipZlibDecompressWorkshop::getOutputStream(){
	return outputStream;
}