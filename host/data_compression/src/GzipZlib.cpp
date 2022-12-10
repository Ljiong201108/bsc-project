#include "GzipZlib.hpp"

namespace dataCompression{
namespace internal{
uint32_t writeGzipHeader(uint8_t *out){
	uint32_t outIdx = 0;
	long int magic_headers = 0x0000000008088B1F;
	std::memcpy(out + outIdx, &magic_headers, 8);
	outIdx += 8;

	long int osheader = 0x00780300;
	std::memcpy(out + outIdx, &osheader, 4);
	outIdx += 4;

	return outIdx;
}

uint32_t writeGzipFooter(uint8_t *out, uint32_t idxAfterCompress, uint32_t checksum, const std::string &fileName, uint32_t fileSize) {
	uint32_t outIdx = idxAfterCompress;

	out[outIdx++] = checksum;
	out[outIdx++] = checksum >> 8;
	out[outIdx++] = checksum >> 16;
	out[outIdx++] = checksum >> 24;

	out[outIdx++] = fileSize;
	out[outIdx++] = fileSize >> 8;
	out[outIdx++] = fileSize >> 16;
	out[outIdx++] = fileSize >> 24;
	return outIdx;
}

uint32_t writeZlibHeader(uint8_t *out){
	uint32_t outIdx = 0;
	uint32_t m_windowbits=15;
	uint32_t m_level=1;
	uint32_t m_strategy=0;

	// Compression method
	uint8_t CM = DEFLATE_METHOD;

	// Compression Window information
	uint8_t CINFO = m_windowbits - 8;

	// Create CMF header
	uint16_t header = (CINFO << 4);
	header |= CM;
	header <<= 8;

	if (m_level < 2 || m_strategy > 2)
		m_level = 0;
	else if (m_level < 6)
		m_level = 1;
	else if (m_level == 6)
		m_level = 2;
	else
		m_level = 3;

	std::cout<<"47 m_windowbits: "<<m_windowbits<<std::endl;
	std::cout<<"48 m_level: "<<m_level<<std::endl;
	std::cout<<"49 m_strategy: "<<m_strategy<<std::endl;

	// CreatE FLG header based on level
	// Strategy information
	header |= (m_level << 6);

	// Ensure that Header (CMF + FLG) is
	// divisible by 31
	header += 31 - (header % 31);

	out[outIdx] = (uint8_t)(header >> 8);
	out[outIdx + 1] = (uint8_t)(header);
	outIdx += 2;

	return outIdx;
}

uint32_t writeZlibFooter(uint8_t *out, uint32_t idxAfterCompress, uint32_t checksum){
	uint32_t outIdx = idxAfterCompress;

	out[outIdx++] = checksum >> 24;
	out[outIdx++] = checksum >> 16;
	out[outIdx++] = checksum >> 8;
	out[outIdx++] = checksum;

	return outIdx;
}

bool readGzipZlibHeader(uint8_t* in){
	uint8_t hidx = 0;
    if (in[hidx++] == 0x1F && in[hidx++] == 0x8B) {
        // Check for magic header
        // Check if method is deflate or not
        if (in[hidx++] != 0x08) {
            std::cerr << "\n";
            std::cerr << "Deflate Header Check Fails" << std::endl;
            return 0;
        }

        // Check if the FLAG has correct value
        // Supported file name or no file name
        // 0x00: No File Name
        // 0x08: File Name
        if (in[hidx] != 0 && in[hidx] != 0x08) {
            std::cerr << "\n";
            std::cerr << "Deflate -n option check failed" << std::endl;
            return 0;
        }
        hidx++;

        // Skip time stamp bytes
        // time stamp contains 4 bytes
        hidx += 4;

        // One extra 0  ending byte
        hidx += 1;

        // Check the operating system code
        // for Unix its 3
        uint8_t oscode_in = in[hidx];
        std::vector<uint8_t> oscodes{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
        bool ochck = std::find(oscodes.cbegin(), oscodes.cend(), oscode_in) == oscodes.cend();
        if (ochck) {
            std::cerr << "\n";
            std::cerr << "GZip header mismatch: OS code is unknown" << std::endl;
            return 0;
        }
    } else {
		hidx = 0;
        // ZLIB Header Checks
        // CMF
        // FLG
        uint8_t cmf = 0x78;
        // 0x01: Fast Mode
        // 0x5E: 1 to 5 levels
        // 0x9C: Default compression: level 6
        // 0xDA: High compression
        std::vector<uint8_t> zlib_flags{0x01, 0x5E, 0x9C, 0xDA};
        if (in[hidx++] == cmf) {
            uint8_t flg = in[hidx];
            bool hchck = std::find(zlib_flags.cbegin(), zlib_flags.cend(), flg) == zlib_flags.cend();
            if (hchck) {
                std::cerr << "\n";
                std::cerr << "Header check fails" << std::endl;
                return 0;
            }
        } else {
            std::cerr << "\n";
            std::cerr << "Zlib Header mismatch" << std::endl;
            return 0;
        }
    }
    return true;
}

uint32_t gzipZlibCompressionInternal1(uint8_t* in, uint8_t* out, uint64_t inputSize, uint64_t maxOutputSize, uint32_t &checksum, bool isZlib){
	cl_int err;

	KernelPointer compressKernelMM(Application::getProgram<Lib::dataCompressionLib>(), "xilGzipZlibCompressMM:{xilGzipZlibCompressMM_1}");

	std::vector<uint32_t, aligned_allocator<uint32_t>> checksumDataBufferHost(1);
	BufferPointer checksumDataBuffer(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), checksumDataBufferHost.data());
	
	bool checksum_type;

    if(isZlib){
        checksumDataBufferHost.data()[0] = 1;
        checksum_type = false;
    } else {
        checksumDataBufferHost.data()[0] = ~0;
        checksum_type = true;
    }

	uint64_t blockNum=inputSize/BLOCK_SIZE_IN_BYTE+(inputSize%BLOCK_SIZE_IN_BYTE!=0);

	// Host Buffers
	std::vector<uint8_t, aligned_allocator<uint8_t>> inputBufferHost(BLOCK_SIZE_IN_BYTE);
	std::vector<uint8_t, aligned_allocator<uint8_t>> outputBufferHost(BLOCK_SIZE_IN_BYTE*MCR);
	std::vector<uint32_t, aligned_allocator<uint32_t>> compressedSizeBufferHost(1);

	// Device buffers
	BufferPointer inputBuffer(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(uint8_t)*inputBufferHost.size(), inputBufferHost.data());
	BufferPointer outputBuffer(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint8_t)*outputBufferHost.size(), outputBufferHost.data());
	BufferPointer compressedSizeBuffer(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint32_t), compressedSizeBufferHost.data());

    compressKernelMM->setArg(0, *inputBuffer);
	compressKernelMM->setArg(1, *outputBuffer);
	compressKernelMM->setArg(2, *compressedSizeBuffer);
	compressKernelMM->setArg(3, *checksumDataBuffer);
	compressKernelMM->setArg(5, checksum_type);

    CommandQueuePointer queue(Application::getContext<Lib::dataCompressionLib>(), Application::getDevice<Lib::dataCompressionLib>(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

    uint64_t outIdx=0;

    // transfer the initial checksum to device
    OCL_CHECK(err, err = queue->enqueueMigrateMemObjects({*checksumDataBuffer}, 0));
	queue->finish();

    // compress the input block by block
    for(uint64_t i=0;i<blockNum;i++){
        uint32_t blockSize=std::min(inputSize-i*BLOCK_SIZE_IN_BYTE, BLOCK_SIZE_IN_BYTE);
        std::memcpy(inputBufferHost.data(), in+i*BLOCK_SIZE_IN_BYTE, blockSize);
        
        // set the kernel input size accordingly
        compressKernelMM->setArg(4, blockSize);

        // transfer host side input buffer to kernel
        OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*inputBuffer}, 0));
	    queue->finish();

        // execute the kernel
        OCL_CHECK(err, err=queue->enqueueTask(*compressKernelMM));
        queue->finish();

        // Migrate the block sizes back
        OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*(compressedSizeBuffer)}, CL_MIGRATE_MEM_OBJECT_HOST));
        queue->finish();

        // test whether host output buffer is big enough to hold the compressed data
        if(outIdx+compressedSizeBufferHost[0]>maxOutputSize){
            std::cout<<"The output buffer is too smaller, please allocate a bigger buffer!"<<std::endl;
            exit(EXIT_FAILURE);
        }

        // transfer the data from device to host
        OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*outputBuffer}, CL_MIGRATE_MEM_OBJECT_HOST));
        queue->finish();

        std::memcpy(out+outIdx, outputBufferHost.data(), compressedSizeBufferHost[0]);
        outIdx+=compressedSizeBufferHost[0];
    }

    // transfer the final checksum to host
    OCL_CHECK(err, err=queue->enqueueMigrateMemObjects({*checksumDataBuffer}, CL_MIGRATE_MEM_OBJECT_HOST));
    queue->finish();

	if(checksum_type) checksumDataBufferHost.data()[0] = ~checksumDataBufferHost.data()[0];

	// Add last block header
	long int last_block = 0xffff000001;
	std::memcpy(&(out[outIdx]), &last_block, 5);
	outIdx += 5;

	checksum = checksumDataBufferHost.data()[0];
	if(checksum_type) checksum = ~checksum;

	return outIdx;
}

uint32_t gzipZlibCompressionInternal(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t &checksum, bool isZlib){
	cl_int err;

	KernelPointer m_compressFullKernel(Application::getProgram<Lib::dataCompressionLib>(), "xilGzipZlibCompressMM:{xilGzipZlibCompressMM_1}");

	std::vector<uint32_t, zlib_aligned_allocator<uint32_t> > h_buf_checksum_data(1);
	BufferPointer buffer_checksum_data(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), h_buf_checksum_data.data());
	
	bool checksum_type;

    if(isZlib){
        h_buf_checksum_data.data()[0] = 1;
        checksum_type = false;
    } else {
        h_buf_checksum_data.data()[0] = ~0;
        checksum_type = true;
    }

	auto BLOCK_SIZE_IN_KB=32;
	auto blockSize = BLOCK_SIZE_IN_KB * 1024; // can be changed for custom testing of block sizes upto 4KB.
	auto blckNum = (inputSize - 1) / blockSize + 1;
	auto output_size = 10 * blckNum * BLOCK_SIZE_IN_KB * 1024;

	// Host Buffers
	std::vector<uint8_t, aligned_allocator<uint8_t> > h_input_buffer(inputSize);
	std::vector<uint8_t, aligned_allocator<uint8_t> > h_output_buffer(output_size);
	std::vector<uint32_t, aligned_allocator<uint32_t> > h_compressSize(2 * blckNum);

	// Device buffers
	BufferPointer buffer_input(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(uint8_t) * inputSize, h_input_buffer.data());
	BufferPointer buffer_output(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint8_t) * output_size, h_output_buffer.data());
	BufferPointer buffer_cSize(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(uint32_t) * blckNum * 2, h_compressSize.data());

	// Copy input data
	std::memcpy(h_input_buffer.data(), in, inputSize);

	// Set kernel arguments
	int narg = 0;

	m_compressFullKernel->setArg(narg++, *buffer_input);
	m_compressFullKernel->setArg(narg++, *buffer_output);
	m_compressFullKernel->setArg(narg++, *buffer_cSize);
	m_compressFullKernel->setArg(narg++, *buffer_checksum_data);
	m_compressFullKernel->setArg(narg++, inputSize);
	m_compressFullKernel->setArg(narg++, checksum_type);

	CommandQueuePointer m_def_q(Application::getContext<Lib::dataCompressionLib>(), Application::getDevice<Lib::dataCompressionLib>(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

	// Transfer the data to device
	OCL_CHECK(err, err = m_def_q->enqueueMigrateMemObjects({*(buffer_input), *(buffer_checksum_data)}, 0, nullptr, nullptr));
	m_def_q->finish();

	// Execute the kernel
	OCL_CHECK(err, err = m_def_q->enqueueTask(*m_compressFullKernel));
	m_def_q->finish();

	// Migrate the block sizes back
	OCL_CHECK(err, err = m_def_q->enqueueMigrateMemObjects({*(buffer_cSize)}, CL_MIGRATE_MEM_OBJECT_HOST));
	m_def_q->finish();
	BOOST_ASSERT_MSG(h_compressSize[0] < unsigned(output_size), "\x1B[35m xGzip Error: Output Buffer Size is not sufficient \033[0m ");

	// Transfer the data from device to host
	OCL_CHECK(err, err = m_def_q->enqueueMigrateMemObjects({*(buffer_output), *(buffer_checksum_data)}, CL_MIGRATE_MEM_OBJECT_HOST));
	m_def_q->finish();

	if(checksum_type) h_buf_checksum_data.data()[0] = ~h_buf_checksum_data.data()[0];

	auto outIdx = 0;
	uint32_t compSizeCntr = h_compressSize[0];
	std::memcpy(out, &h_output_buffer[0], compSizeCntr);
	outIdx += compSizeCntr;

	// Add last block header
	long int last_block = 0xffff000001;
	std::memcpy(&(out[outIdx]), &last_block, 5);
	outIdx += 5;

	checksum = h_buf_checksum_data.data()[0];
	if(checksum_type) checksum = ~checksum;

	return outIdx;
}

uint32_t gzipZlibDecompressionInternalMM(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t max_outbuf_size){
	cl_int err;
    // Streaming based solution

    uint32_t inBufferSize = inputSize;
    const uint32_t lim_4gb = (uint32_t)(((uint64_t)1024 * 1024 * 1024) - 2); // 4GB limit on output size
    uint32_t outBufferSize = 0;
    // auto num_itr = 1;

    if (max_outbuf_size > lim_4gb) {
        outBufferSize = lim_4gb;
    } else {
        outBufferSize = (uint32_t)max_outbuf_size;
    }

	// host allocated aligned memory
    std::vector<uint8_t, aligned_allocator<uint8_t> > dbuf_in(inBufferSize);
    std::vector<uint8_t, aligned_allocator<uint8_t> > dbuf_out(outBufferSize);
    std::vector<uint32_t, aligned_allocator<uint32_t> > dbuf_outSize(2);

	BufferPointer buffer_dec_zlib_output(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, outBufferSize, dbuf_out.data());
	BufferPointer buffer_dec_input(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, inBufferSize, dbuf_in.data());
	BufferPointer buffer_size(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), dbuf_outSize.data());
	
	KernelPointer decompress_kernel(Application::getProgram<Lib::dataCompressionLib>(), "xilGzipZlibDecompressMM:{xilGzipZlibDecompressMM_1}");
	decompress_kernel->setArg(0, *buffer_dec_input);
    decompress_kernel->setArg(1, *buffer_dec_zlib_output);
    decompress_kernel->setArg(2, *buffer_size);
    decompress_kernel->setArg(3, inBufferSize);

	// Copy input data
    std::memcpy(dbuf_in.data(), in, inBufferSize); // must be equal to inputSize
	
	CommandQueuePointer m_q_dec(Application::getContext<Lib::dataCompressionLib>(), Application::getDevice<Lib::dataCompressionLib>(), CL_QUEUE_PROFILING_ENABLE);
    OCL_CHECK(err, err = m_q_dec->enqueueMigrateMemObjects({*(buffer_dec_input)}, 0, NULL, NULL));
    m_q_dec->finish();

	m_q_dec->enqueueTask(*decompress_kernel);
    m_q_dec->finish();

	// copy decompressed output data
    m_q_dec->enqueueMigrateMemObjects({*(buffer_size), *(buffer_dec_zlib_output)}, CL_MIGRATE_MEM_OBJECT_HOST, NULL, NULL);
    m_q_dec->finish();

	// decompressed size
    uint32_t decmpSizeIdx = dbuf_outSize[0];
    // copy output decompressed data
    std::memcpy(out, dbuf_out.data(), decmpSizeIdx);

    return decmpSizeIdx;
}

uint32_t gzipZlibDecompressionInternalStream(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t max_outbuf_size){
	cl_int err;
    // Streaming based solution

    uint32_t inBufferSize = inputSize;
    uint32_t isLast = 1;
    const uint32_t lim_4gb = (uint32_t)(((uint64_t)1024 * 1024 * 1024) - 2); // 4GB limit on output size
    uint32_t outBufferSize = 0;
	std::vector<uint32_t, aligned_allocator<uint32_t>> h_dcompressStatus(4);
    h_dcompressStatus.data()[0] = 0;
    if (max_outbuf_size > lim_4gb) {
        outBufferSize = lim_4gb;
    } else {
        outBufferSize = (uint32_t)max_outbuf_size;
    }

	// host allocated aligned memory
    std::vector<uint8_t, aligned_allocator<uint8_t> > dbuf_in(inBufferSize);
    std::vector<uint8_t, aligned_allocator<uint8_t> > dbuf_out(outBufferSize);
    std::vector<uint32_t, aligned_allocator<uint32_t> > dbuf_outSize(2);

	BufferPointer buffer_dec_zlib_output(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, outBufferSize, dbuf_out.data());
	BufferPointer buffer_dec_input(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, inBufferSize, dbuf_in.data());
	BufferPointer buffer_size(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), dbuf_outSize.data());
	BufferPointer buffer_status(Application::getContext<Lib::dataCompressionLib>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t), h_dcompressStatus.data());

	KernelPointer data_writer_kernel(Application::getProgram<Lib::dataCompressionLib>(), "xilMM2S:{xilMM2S_1}");
	KernelPointer data_reader_kernel(Application::getProgram<Lib::dataCompressionLib>(), "xilS2MM:{xilS2MM_1}");

	data_writer_kernel->setArg(0, *(buffer_dec_input));
    data_writer_kernel->setArg(1, inBufferSize);
    data_writer_kernel->setArg(2, isLast);

    data_reader_kernel->setArg(0, *(buffer_dec_zlib_output));
    data_reader_kernel->setArg(1, *(buffer_size));
    data_reader_kernel->setArg(2, *(buffer_status));
    data_reader_kernel->setArg(3, outBufferSize);

	uint32_t decmpSizeIdx = 0;
    h_dcompressStatus.data()[0] = 0;

	CommandQueuePointer m_q_wr(Application::getContext<Lib::dataCompressionLib>(), Application::getDevice<Lib::dataCompressionLib>(), CL_QUEUE_PROFILING_ENABLE);
	CommandQueuePointer m_q_rd(Application::getContext<Lib::dataCompressionLib>(), Application::getDevice<Lib::dataCompressionLib>(), CL_QUEUE_PROFILING_ENABLE);

	// Copy input data
    std::memcpy(dbuf_in.data(), in, inBufferSize); // must be equal to inputSize
    OCL_CHECK(err, err = m_q_wr->enqueueMigrateMemObjects({*(buffer_dec_input)}, 0, NULL, NULL));
    m_q_wr->finish();

    OCL_CHECK(err, err = m_q_rd->enqueueMigrateMemObjects({*(buffer_status)}, 0, NULL, NULL));
    m_q_rd->finish();

	// enqueue data movers
    m_q_rd->enqueueTask(*data_reader_kernel);

    sleep(1);
    // enqueue decompression kernel
    m_q_wr->enqueueTask(*data_writer_kernel);
    m_q_wr->finish();

	// wait for reader to finish
    m_q_rd->finish();

    // copy decompressed output data
    m_q_rd->enqueueMigrateMemObjects({*(buffer_size), *(buffer_status)}, CL_MIGRATE_MEM_OBJECT_HOST, NULL, NULL);
    m_q_rd->finish();

	// decompressed size
    decmpSizeIdx = dbuf_outSize[0];
    if (decmpSizeIdx >= max_outbuf_size) {
        auto brkloop = false;
        do {
            m_q_rd->enqueueTask(*data_reader_kernel);
            // wait for reader to finish
            m_q_rd->finish();

            // copy decompressed output data
            m_q_rd->enqueueMigrateMemObjects({*(buffer_size), *(buffer_status)}, CL_MIGRATE_MEM_OBJECT_HOST, NULL,
                                                NULL);
            m_q_rd->finish();

            decmpSizeIdx += dbuf_outSize[0];
            brkloop = h_dcompressStatus.data()[0];
        } while (!brkloop);
        std::cout << "\n" << std::endl;
        std::cout << "compressed size (.gz/.xz): " << inputSize << std::endl;
        std::cout << "decompressed size : " << decmpSizeIdx << std::endl;
        std::cout << "maximum output buffer allocated: " << max_outbuf_size << std::endl;
        std::cout << "Output Buffer Size Exceeds as the Compression Ratio is High " << std::endl;
        std::cout << "Use -mcr option to increase the output buffer size (Default: 20) --> Aborting " << std::endl;
        abort();
    } else {
        m_q_rd->enqueueMigrateMemObjects({*(buffer_dec_zlib_output)}, CL_MIGRATE_MEM_OBJECT_HOST, NULL, NULL);
        m_q_rd->finish();
        // copy output decompressed data
        std::memcpy(out, dbuf_out.data(), decmpSizeIdx);
    }

    return decmpSizeIdx;
}
} //namespace internal

uint32_t gzipZlibCompression(uint8_t *in, uint8_t *out, uint32_t inputSize, const std::string &fileName, bool isZlib){
	std::cout<<"Original: "<<std::endl;
	hexdump(in, inputSize);
	uint32_t checksum=0;
	uint32_t outIdx=isZlib ? internal::writeZlibHeader(out) : internal::writeGzipHeader(out);
	outIdx+=internal::gzipZlibCompressionInternal1(in, out+outIdx, inputSize, inputSize*20, checksum, isZlib);
	outIdx=isZlib ? internal::writeZlibFooter(out, outIdx, checksum) : internal::writeGzipFooter(out, outIdx, checksum, fileName, inputSize);
	std::cout<<"Compressed: "<<std::endl;
	hexdump(out, outIdx);
	std::cout<<"checksum: "<<std::hex<<checksum<<std::dec<<std::endl;
	return outIdx;
}

uint32_t gzipZlibDecompression(uint8_t* in, uint8_t* out, uint32_t inputSize, uint32_t maxOutputSize, bool stream){
	std::cout<<"Original: "<<std::endl;
	hexdump(in, inputSize);
	bool hcheck = internal::readGzipZlibHeader(in);
    if (!hcheck) {
        std::cerr << "Header Check Failed" << std::endl;
        return 0;
    }
	uint32_t outIdx = stream ? internal::gzipZlibDecompressionInternalStream(in, out, inputSize, maxOutputSize) : internal::gzipZlibDecompressionInternalMM(in, out, inputSize, maxOutputSize);
	std::cout<<"Decompressed: "<<std::endl;
	hexdump(out, outIdx);
	return outIdx;
}
} //namespace dataCompression