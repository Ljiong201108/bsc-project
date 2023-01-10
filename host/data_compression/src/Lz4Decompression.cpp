#include "Lz4Decompression.hpp"
#include <sstream>
#include <bitset>
#include "DataCompression.hpp"

void Lz4DecompressionKernelExecutor::process(){
	cl_int err;

	uint64_t hostBufferSize = Lz4DecompressionWorkshop::HOST_BUFFER_SIZE;
	std::vector<uint8_t, aligned_allocator<uint8_t>> inBufferHost(hostBufferSize);
	std::vector<uint8_t, aligned_allocator<uint8_t>> outBufferHost(hostBufferSize);

	CommandQueuePointer queue(Application::getContext(), Application::getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
	std::stringstream lz4DecompressKernelName;
	lz4DecompressKernelName<<"xilLz4DecompressMM:{xilLz4DecompressMM_"<<idx<<"}";
	KernelPointer lz4DecompressKernel(Application::getProgram<Lib::LZ4_DECOMPRESSION>(), lz4DecompressKernelName.str());

	// Device buffer allocation
	BufferPointer inputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, hostBufferSize, inBufferHost.data());
	BufferPointer outputBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, hostBufferSize, outBufferHost.data());

	lz4DecompressKernel->setArg(0, *inputBuffer);
	lz4DecompressKernel->setArg(1, *outputBuffer);

	// std::vector<cl::UserEvent> inputEvents;
	// std::vector<cl::UserEvent> migrationH2DEvents;
	// std::vector<cl::UserEvent> kernelExeutionEvents;
	// std::vector<cl::UserEvent> migrationD2HEvents;
	// std::vector<cl::UserEvent> outputEvents;

	// std::vector<std::vector<cl::Event>> migrationH2DEventsDep;
	// std::vector<std::vector<cl::Event>> kernelExeutionEventsDep;
	// std::vector<std::vector<cl::Event>> migrationD2HEventsDep;

	// std::vector<std::thread> outputThreads;

	uint64_t curItem=0;

	while(true){
		// if(curItem>0) migrationH2DEvents[curItem-1].wait();

		Lz4DecompressionItem item=inputStream.pop();
		uint64_t itemIdx=item.idx;

		std::cout<<"kernel executor "<<idx<<" get an Item with idx="<<itemIdx<<" numBlocks="<<item.numBlocks<<" blockSizeInBytes="<<item.blockSizeInBytes<<std::endl;
		if(itemIdx==(uint64_t)-1) break;

		// OCL_CHECK(err, inputEvents.emplace_back(Application::getContext(), &err))
		// OCL_CHECK(err, migrationH2DEvents.emplace_back(Application::getContext(), &err))
		// OCL_CHECK(err, kernelExeutionEvents.emplace_back(Application::getContext(), &err))
		// OCL_CHECK(err, migrationD2HEvents.emplace_back(Application::getContext(), &err))
		// OCL_CHECK(err, outputEvents.emplace_back(Application::getContext(), &err))

		// migrationH2DEventsDep.emplace_back();
		// kernelExeutionEventsDep.emplace_back();
		// migrationD2HEventsDep.emplace_back();

		uint32_t numBlocks=item.numBlocks;
		uint32_t blockSizeInBytes=item.blockSizeInBytes;
		uint32_t maxNumBlocks=hostBufferSize/blockSizeInBytes;

		std::vector<uint32_t, aligned_allocator<uint32_t>> decompressedSizeBufferHost(maxNumBlocks);
		std::vector<uint32_t, aligned_allocator<uint32_t>> compressedSizeBufferHost(maxNumBlocks);

		std::memcpy(compressedSizeBufferHost.data(), item.compressedSize, sizeof(uint32_t)*numBlocks);
		std::memcpy(inBufferHost.data(), item.payload, numBlocks*blockSizeInBytes);
		delete[] item.compressedSize;
		delete[] item.payload;

		BufferPointer decompressdSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t)*maxNumBlocks, decompressedSizeBufferHost.data());
		BufferPointer compressdSizeBuffer(Application::getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t)*maxNumBlocks, compressedSizeBufferHost.data());
		// OCL_CHECK(err, err=inputEvents[curItem].setStatus(CL_COMPLETE));

		// migrationH2DEventsDep[curItem].push_back(inputEvents[curItem]);
		// if(curItem>0) migrationH2DEventsDep[curItem].push_back(kernelExeutionEvents[curItem-1]);
		// OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*inputBuffer, *compressdSizeBuffer}, 0, &migrationH2DEventsDep[curItem], &migrationH2DEvents[curItem]))
		queue->enqueueMigrateMemObjects({*inputBuffer, *compressdSizeBuffer}, 0);
		queue->finish();

		lz4DecompressKernel->setArg(2, *decompressdSizeBuffer);
		lz4DecompressKernel->setArg(3, *compressdSizeBuffer);
		lz4DecompressKernel->setArg(4, blockSizeInBytes/1024);
		lz4DecompressKernel->setArg(5, numBlocks);
		// kernelExeutionEventsDep[curItem].push_back(migrationH2DEvents[curItem]);
		// if(curItem>0) kernelExeutionEventsDep[curItem].push_back(migrationD2HEvents[curItem-1]);
		// OCL_CHECK(err, err=queue->enqueueTask(*lz4DecompressKernel, &kernelExeutionEventsDep[curItem], &kernelExeutionEvents[curItem]))
		queue->enqueueTask(*lz4DecompressKernel);
		queue->finish();

		// migrationD2HEventsDep[curItem].push_back(kernelExeutionEvents[curItem]);
		// if(curItem>0) migrationD2HEventsDep[curItem].push_back(outputEvents[curItem-1]);
		// OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*decompressdSizeBuffer, *outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST, &migrationD2HEventsDep[curItem], &migrationD2HEvents[curItem]))
		queue->enqueueMigrateMemObjects({*decompressdSizeBuffer, *outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST);
		queue->finish();

		// outputThreads.emplace_back([this, numBlocks, &decompressedSizeBufferHost, &outBufferHost, &migrationD2HEvents, curItem, &outputEvents, itemIdx]{
		// 	migrationD2HEvents[curItem].wait();
			std::cout<<"going to push item "<<itemIdx<<std::endl;
			std::cout<<"It's No. "<<curItem<<" of this executor with "<<numBlocks<<" blocks"<<std::endl;
			std::unique_lock<std::mutex> lck(workshop.mtx);
			workshop.cv.wait(lck, [this, itemIdx] {return workshop.outputIdx==itemIdx;});

			for(uint32_t i=0;i<numBlocks;i++){
				std::cout<<i<<" "<<decompressedSizeBufferHost[i]<<std::endl;
			}

			for(uint32_t blockIdx=0, decompressedSize=0;blockIdx<numBlocks;blockIdx++){
				uint32_t blockSize=decompressedSizeBufferHost[blockIdx];
				outputStream.push(outBufferHost.data()+decompressedSize, blockSize, false);
				std::cout<<"decompressed a block"<<std::endl;
				decompressedSize+=blockSize;
			}

			// OCL_CHECK(err, cl_int err=outputEvents[curItem].setStatus(CL_COMPLETE))
			workshop.outputIdx++;
			workshop.cv.notify_all();
			std::cout<<"Finished pushing item "<<itemIdx<<std::endl;
		// });

		curItem++;
	}

	// for(auto &t : outputThreads) t.join();
	// for(auto &e : inputEvents) e.wait();
	// for(auto &e : migrationH2DEvents) e.wait();
	// for(auto &e : kernelExeutionEvents) e.wait();
	// for(auto &e : migrationD2HEvents) e.wait();
	// for(auto &e : outputEvents) e.wait();

	std::cout<<"End of kernel executor "<<idx<<std::endl;
}

Lz4DecompressionKernelExecutor::Lz4DecompressionKernelExecutor(
	int idx,
	Stream<Lz4DecompressionItem> &inputStream, 
	ByteStream &outputStream,  Lz4DecompressionWorkshop &workshop) 
	: KernelExecutor<Lz4DecompressionItem>(idx, inputStream, outputStream), workshop(workshop){}

void Lz4DecompressionStructureAnalyzer::process(){
	bool last;
	uint64_t idx=0;
	
	do{ // per frames
		uint32_t magnicNumber;
		inputStream.pop(&magnicNumber, 4, last);
		std::cout<<std::hex<<"magic number: "<<magnicNumber<<std::endl;
		if(magnicNumber==0x184D2204){
			//General Structure of LZ4 Frame format
			std::cout<<"General Structure of LZ4 Frame format"<<std::endl;

			//FLG
			uint8_t flg;
			inputStream.pop(&flg, 1, last);
			std::cout<<(uint32_t)flg<<std::endl;
			uint8_t dictID=flg&0x1;
			std::cout<<"dictID: "<<(uint32_t)dictID<<std::endl;
			if(dictID){
				std::cerr<<"Cannot decompress where dictID is set!"<<std::endl;
				exit(EXIT_FAILURE);
			}
			flg>>=2;

			uint8_t contentChecksumFlg=flg&0x1;
			std::cout<<"contentChecksumFlg: "<<(uint32_t)contentChecksumFlg<<std::endl;
			flg>>=1;

			uint8_t contentSizeFlg=flg&0x1;
			std::cout<<"contentSizeFlg: "<<(uint32_t)contentSizeFlg<<std::endl;
			flg>>=1;

			uint8_t blockChecksumFlg=flg&0x1;
			std::cout<<"blockChecksumFlg: "<<(uint32_t)blockChecksumFlg<<std::endl;
			flg>>=1;

			uint8_t blockIndep=flg&0x1;
			std::cout<<"blockIndep: "<<(uint32_t)blockIndep<<std::endl;
			if(!blockIndep){
				std::cerr<<"Cannot decompress where block independence is not set!"<<std::endl;
				exit(EXIT_FAILURE);
			}
			flg>>=1;

			if(flg!=0b01){
				std::cout<<"version: "<<(uint32_t)flg<<std::endl;
				std::cerr<<"version incorrect!"<<std::endl;
				exit(EXIT_FAILURE);
			}

			//BD
			uint8_t bd;
			inputStream.pop(&bd, 1, last);
			std::cout<<"BD: "<<(uint32_t)bd<<std::endl;
			bd>>=4;
			uint32_t blockMaxSizeFlg=bd&0b111;
			uint32_t blockSizeInBytes;

			switch(blockMaxSizeFlg){
				case 4: blockSizeInBytes=64*1024; break;
				case 5: blockSizeInBytes=256*1024; break;
				case 6: blockSizeInBytes=1*1024*1024; break;
				case 7: blockSizeInBytes=4*1024*1024; break;
				default: std::cerr<<"Block Max Size FLG invalid!"<<std::endl; exit(EXIT_FAILURE);
			}
			std::cout<<"blockSizeInBytes: "<<blockSizeInBytes<<std::endl;
			
			//content size
			if(contentSizeFlg){
				uint32_t contentSize;
				inputStream.pop(&contentSize, 4, last);
			}

			//HC
			uint8_t HC;
			inputStream.pop(&HC, 1, last);

			uint32_t maxNumBlocks=Lz4DecompressionWorkshop::HOST_BUFFER_SIZE/blockSizeInBytes;
			std::vector<uint8_t> inBufferHost(Lz4DecompressionWorkshop::HOST_BUFFER_SIZE);
			std::vector<uint32_t> compressedSizeBufferHost(maxNumBlocks);
			//data blocks
			inputStream.pop(inBufferHost.data(), 4, last);

			uint32_t bufferTotalBlock=0;

			while(*(uint32_t*)(inBufferHost.data()+bufferTotalBlock*blockSizeInBytes)!=0x0){
				uint32_t compressedSize=*(uint32_t*)(inBufferHost.data()+bufferTotalBlock*blockSizeInBytes);
				if(compressedSize>>31) compressedSize=compressedSize&0x7FFFFFFF;
				std::cout<<"trying to read a block, size: "<<std::dec<<compressedSize<<std::endl;

				compressedSizeBufferHost[bufferTotalBlock]=compressedSize;
				inputStream.pop(inBufferHost.data()+bufferTotalBlock*blockSizeInBytes, compressedSize, last);
				bufferTotalBlock++;

				//这里必须要减一，不然decompressedSize数据前面全是0，就最后一个是65536
				if(bufferTotalBlock==maxNumBlocks-1){
					uint32_t *compressedSize=new uint32_t[bufferTotalBlock];
					uint8_t *payload=new uint8_t[bufferTotalBlock*blockSizeInBytes];
					memcpy(compressedSize, compressedSizeBufferHost.data(), bufferTotalBlock);
					memcpy(payload, inBufferHost.data(), bufferTotalBlock*blockSizeInBytes);

					Lz4DecompressionItem item;
					item.idx=idx++;
					item.numBlocks=bufferTotalBlock;
					item.blockSizeInBytes=blockSizeInBytes;
					item.compressedSize=compressedSize;
					item.payload=payload;
					outputStream.push(item);

					bufferTotalBlock=0;
				}
				
				uint32_t checksum;
				if(blockChecksumFlg) inputStream.pop(&checksum, 4, last);
				
				std::cout<<"finished reading a block"<<std::endl;
				//next data block size
				inputStream.pop(inBufferHost.data()+bufferTotalBlock*blockSizeInBytes, 4, last);
			}

			if(bufferTotalBlock){
				uint32_t *compressedSize=new uint32_t[bufferTotalBlock];
				uint8_t *payload=new uint8_t[bufferTotalBlock*blockSizeInBytes];
				memcpy(compressedSize, compressedSizeBufferHost.data(), sizeof(uint32_t)*bufferTotalBlock);
				memcpy(payload, inBufferHost.data(), bufferTotalBlock*blockSizeInBytes);

				Lz4DecompressionItem item;
				item.idx=idx++;
				item.numBlocks=bufferTotalBlock;
				item.blockSizeInBytes=blockSizeInBytes;
				item.compressedSize=compressedSize;
				item.payload=payload;
				outputStream.push(item);
			}

			if(contentChecksumFlg) inputStream.pop(inBufferHost.data(), 4, last);
		}else{
			std::cerr<<"Magic Number error"<<std::endl;
			exit(EXIT_FAILURE);
		}

		std::cout<<"frame end"<<std::endl;
	}while(!last);

	std::cout<<"End of structure analyzer"<<std::endl;
}

Lz4DecompressionStructureAnalyzer::Lz4DecompressionStructureAnalyzer(
		ByteStream &inputStream,
		Stream<Lz4DecompressionItem> &outputStream) : 
		StructureAnalyzer<Lz4DecompressionItem>(inputStream, outputStream){}

Lz4DecompressionWorkshop::Lz4DecompressionWorkshop() : 
	Workshop<Lz4DecompressionItem>("Lz4InputStream", 4, "Lz4BridgeStream", 8, "Lz4OutputStream", 4), 
	kernelExecutor1(1, bridgeStream, outputStream, *this),
	// kernelExecutor2(2, bridgeStream, outputStream, *this),
	// kernelExecutor3(3, bridgeStream, outputStream, *this),
	// kernelExecutor4(4, bridgeStream, outputStream, *this),
	// kernelExecutor5(5, bridgeStream, outputStream, *this),
	// kernelExecutor6(6, bridgeStream, outputStream, *this),
	// kernelExecutor7(7, bridgeStream, outputStream, *this),
	// kernelExecutor8(8, bridgeStream, outputStream, *this),
	structureAnalyzer(inputStream, bridgeStream),
	structureAnalyzerThread(&Lz4DecompressionStructureAnalyzer::process, &structureAnalyzer),
	kernelExecutorThread1(&Lz4DecompressionKernelExecutor::process, &kernelExecutor1)
	// kernelExecutorThread2(&Lz4DecompressionKernelExecutor::process, &kernelExecutor2),
	// kernelExecutorThread3(&Lz4DecompressionKernelExecutor::process, &kernelExecutor3),
	// kernelExecutorThread4(&Lz4DecompressionKernelExecutor::process, &kernelExecutor4),
	// kernelExecutorThread5(&Lz4DecompressionKernelExecutor::process, &kernelExecutor5),
	// kernelExecutorThread6(&Lz4DecompressionKernelExecutor::process, &kernelExecutor6),
	// kernelExecutorThread7(&Lz4DecompressionKernelExecutor::process, &kernelExecutor7),
	// kernelExecutorThread8(&Lz4DecompressionKernelExecutor::process, &kernelExecutor8)
	{}

void Lz4DecompressionWorkshop::wait(){
	structureAnalyzerThread.join();

	std::cout<<"structureAnalyzerThread is joined"<<std::endl;

	Lz4DecompressionItem item;
	item.idx=(uint64_t)-1;
	item.compressedSize=nullptr;
	item.payload=nullptr;

	bridgeStream.push(item);
	bridgeStream.push(item);

	std::cout<<"break point 0"<<std::endl;

	kernelExecutorThread1.join();
	std::cout<<"break point 1"<<std::endl;
	// kernelExecutorThread2.join();
	std::cout<<"break point 2"<<std::endl;
	// kernelExecutorThread3.join();
	// kernelExecutorThread4.join();
	// kernelExecutorThread5.join();
	// kernelExecutorThread6.join();
	// kernelExecutorThread7.join();
	// kernelExecutorThread8.join();

	uint8_t tmp;
	outputStream.push(&tmp, 1, true);

	std::cout<<"kernelExecutorThread is joined"<<std::endl;
}

ByteStream& Lz4DecompressionWorkshop::getInputStream(){
	return inputStream;
}

ByteStream& Lz4DecompressionWorkshop::getOutputStream(){
	return outputStream;
}