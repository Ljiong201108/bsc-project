#include "Lz4.hpp"

namespace dataCompression{
namespace internal{
uint8_t writeLz4Header(uint8_t* out, size_t input_size) {
    uint8_t fileIdx = 0;
    (out[fileIdx++]) = xf::compression::MAGIC_BYTE_1;
    (out[fileIdx++]) = xf::compression::MAGIC_BYTE_2;
    (out[fileIdx++]) = xf::compression::MAGIC_BYTE_3;
    (out[fileIdx++]) = xf::compression::MAGIC_BYTE_4;

    // FLG & BD bytes
    // --no-frame-crc flow
    // --content-size
    (out[fileIdx++]) = xf::compression::FLG_BYTE;

    size_t block_size_header = 0;
    // Default value 64K
    switch (BLOCK_SIZE_IN_KB) {
        case 64:
            (out[fileIdx++]) = xf::compression::BSIZE_STD_64KB;
            block_size_header = xf::compression::BSIZE_STD_64KB;
            break;
        case 256:
            (out[fileIdx++]) = xf::compression::BSIZE_STD_256KB;
            block_size_header = xf::compression::BSIZE_STD_256KB;
            break;
        case 1024:
            (out[fileIdx++]) = xf::compression::BSIZE_STD_1024KB;
            block_size_header = xf::compression::BSIZE_STD_1024KB;
            break;
        case 4096:
            (out[fileIdx++]) = xf::compression::BSIZE_STD_4096KB;
            block_size_header = xf::compression::BSIZE_STD_4096KB;
            break;
        default:
            std::cout << "Invalid Block Size" << std::endl;
            break;
    }

    uint8_t temp_buff[10] = {xf::compression::FLG_BYTE,
                             (uint8_t)block_size_header,
                             (uint8_t)input_size,
                             (uint8_t)(input_size >> 8),
                             (uint8_t)(input_size >> 16),
                             (uint8_t)(input_size >> 24),
                             (uint8_t)(input_size >> 32),
                             (uint8_t)(input_size >> 40),
                             (uint8_t)(input_size >> 48),
                             (uint8_t)(input_size >> 56)};

    // xxhash is used to calculate hash value
    uint32_t xxh = XXH32(temp_buff, 2, 0);

    //  Header CRC
    out[fileIdx++] = (uint8_t)(xxh >> 8);

    return fileIdx;
}

uint8_t writeLz4Footer(uint8_t* in, uint8_t* out, uint64_t input_size) {
    uint8_t fileIdx = 0;
    uint32_t* zero_ptr = 0;
    memcpy(out, &zero_ptr, 4);
    fileIdx += 4;

    // xxhash is used to calculate content checksum value
    uint32_t xxh = XXH32(in, input_size, 0);
    memcpy(out+fileIdx, &xxh, 4);
    fileIdx += 4;
    return fileIdx;
}

uint8_t readLz4Header(uint8_t* in) {
    uint8_t fileIdx = 0;
    // Read magic header 4 bytes
    char c = 0;
    int magic_hdr[] = {xf::compression::MAGIC_BYTE_1, xf::compression::MAGIC_BYTE_2, xf::compression::MAGIC_BYTE_3, xf::compression::MAGIC_BYTE_4};
    for (uint32_t i = 0; i < xf::compression::MAGIC_HEADER_SIZE; i++) {
        // inFile.get(v);
        c = in[fileIdx++];
        if (int(c) == magic_hdr[i])
            continue;
        else {
            std::cerr << "Problem with magic header " << c << " " << i << std::endl;
            exit(1);
        }
    }

    // FLG Byte
    c = in[fileIdx++];

    // Check if block size is 64 KB
    c = in[fileIdx++];

    // switch (c) {
    //     case xf::compression::BSIZE_STD_64KB:
    //         m_BlockSizeInKb = 64;
    //         break;
    //     case xf::compression::BSIZE_STD_256KB:
    //         m_BlockSizeInKb = 256;
    //         break;
    //     case xf::compression::BSIZE_STD_1024KB:
    //         m_BlockSizeInKb = 1024;
    //         break;
    //     case xf::compression::BSIZE_STD_4096KB:
    //         m_BlockSizeInKb = 4096;
    //         break;
    //     default:
    //         std::cout << "Invalid Block Size" << std::endl;
    //         break;
    // }

    if (xf::compression::FLG_BYTE == 104) {
        // Original size
        size_t original_size = 0;

        memcpy(&original_size, &in[fileIdx], 8);

        // file size(8)
        fileIdx += 8;
    }

    // Header Checksum
    c = in[fileIdx++];

    // m_HostBufferSize = (m_BlockSizeInKb * 1024) * MAX_NUMBER_BLOCKS;

    return fileIdx;
}

uint64_t lz4CompressEngineMM(uint8_t* in, uint8_t* out, size_t input_size) {
    uint32_t host_buffer_size = HOST_BUFFER_SIZE;
    uint32_t max_num_blks = (host_buffer_size) / (BLOCK_SIZE_IN_KB * 1024);
    std::vector<uint8_t, aligned_allocator<uint8_t>> h_buf_in(host_buffer_size);
    std::vector<uint8_t, aligned_allocator<uint8_t>> h_buf_out(host_buffer_size);
    std::vector<uint32_t, aligned_allocator<uint32_t>> h_blksize(max_num_blks);
    std::vector<uint32_t, aligned_allocator<uint32_t>> h_compressSize(max_num_blks);

    uint32_t block_size_in_kb = BLOCK_SIZE_IN_KB;
    uint32_t block_size_in_bytes = block_size_in_kb * 1024;

    // Keeps track of output buffer index
    uint64_t outIdx = 0;

    // Given a input file, we process it as multiple chunks
    // Each compute unit is assigned with a chunk of data
    // In this example HOST_BUFFER_SIZE is the chunk size.
    // For example: Input file = 12 MB
    // HOST_BUFFER_SIZE = 2MB
    // Each compute unit processes 2MB data per kernel invocation
    uint32_t hostChunk_cu;

    // This buffer contains total number of m_BlockSizeInKb blocks per CU
    // For Example: HOST_BUFFER_SIZE = 2MB/m_BlockSizeInKb = 32block (Block
    // size 64 by default)
    uint32_t total_blocks_cu;

    // This buffer holds exact size of the chunk in bytes for all the CUs
    uint32_t bufSize_in_bytes_cu;

    // Holds value of total compute units to be
    // used per iteration
    uint32_t compute_cu = 0;

    cl::CommandQueue *m_q = new cl::CommandQueue(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
    cl::Kernel *compress_kernel_lz4 = new cl::Kernel(Application::getInstance().getProgram(), "xilLz4CompressMM:{xilLz4CompressMM_1}");

    for (uint64_t inIdx = 0; inIdx < input_size; inIdx += host_buffer_size) {
        // Needs to reset this variable
        // As this drives compute unit launch per iteration
        compute_cu = 0;
        // Pick buffer size as predefined one
        // If yet to be consumed input is lesser
        // the reset to required size
        uint32_t buf_size = host_buffer_size;
        // This loop traverses through each compute based current inIdx
        // It tries to calculate chunk size and total compute units need to be
        // launched (based on the input_size)
        hostChunk_cu = 0;
        // If amount of data to be consumed is less than HOST_BUFFER_SIZE
        // Then choose to send is what is needed instead of full buffer size
        // based on host buffer macro
        if (inIdx + (buf_size) > input_size) {
            hostChunk_cu = input_size - (inIdx);
            compute_cu++;
        } else {
            hostChunk_cu = buf_size;
            compute_cu++;
        }
        // Figure out total number of blocks need per each chunk
        // Copy input data from in to host buffer based on the inIdx and cu
        uint32_t nblocks = (hostChunk_cu - 1) / block_size_in_bytes + 1;
        total_blocks_cu = nblocks;
        std::memcpy(h_buf_in.data(), &in[inIdx], hostChunk_cu);
        // Fill the host block size buffer with various block sizes per chunk/cu
        uint32_t bIdx = 0;
        uint32_t chunkSize_curr_cu = hostChunk_cu;

        for (uint32_t bs = 0; bs < chunkSize_curr_cu; bs += block_size_in_bytes) {
            uint32_t block_size = block_size_in_bytes;
            if (bs + block_size > chunkSize_curr_cu) {
                block_size = chunkSize_curr_cu - bs;
            }
            h_blksize.data()[bIdx++] = block_size;
        }
        // Calculate chunks size in bytes for device buffer creation
        bufSize_in_bytes_cu = ((hostChunk_cu - 1) / BLOCK_SIZE_IN_KB + 1) * BLOCK_SIZE_IN_KB;

        // Device buffer allocation
        cl::Buffer *buffer_input =
            new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, bufSize_in_bytes_cu, h_buf_in.data());
        cl::Buffer *buffer_output =
            new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, bufSize_in_bytes_cu, h_buf_out.data());
        cl::Buffer *buffer_compressed_size = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                                                sizeof(uint32_t) * total_blocks_cu, h_compressSize.data());
        cl::Buffer *buffer_block_size = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                                           sizeof(uint32_t) * total_blocks_cu, h_blksize.data());

        // Set kernel arguments
        uint32_t narg = 0;
        compress_kernel_lz4->setArg(narg++, *(buffer_input));
        compress_kernel_lz4->setArg(narg++, *(buffer_output));
        compress_kernel_lz4->setArg(narg++, *(buffer_compressed_size));
        compress_kernel_lz4->setArg(narg++, *(buffer_block_size));
        compress_kernel_lz4->setArg(narg++, block_size_in_kb);
        compress_kernel_lz4->setArg(narg++, hostChunk_cu);
        std::vector<cl::Memory> inBufVec;

        inBufVec.push_back(*(buffer_input));
        inBufVec.push_back(*(buffer_block_size));

        // Migrate memory - Map host to device buffers
        m_q->enqueueMigrateMemObjects(inBufVec, 0 /* 0 means from host*/);
        m_q->finish();

        // Fire kernel execution
        m_q->enqueueTask(*compress_kernel_lz4);
        // Wait till kernels complete
        m_q->finish();
        
        // Setup output buffer vectors
        std::vector<cl::Memory> outBufVec;
        outBufVec.push_back(*(buffer_output));
        outBufVec.push_back(*(buffer_compressed_size));
        // Migrate memory - Map device to host buffers
        m_q->enqueueMigrateMemObjects(outBufVec, CL_MIGRATE_MEM_OBJECT_HOST);
        m_q->finish();

        // Copy data into out buffer
        // Include compress and block size data
        // Copy data block by block within a chunk example 2MB (64block size) - 32 blocks data
        // Do the same for all the compute units
        writeCompressedBlock<uint8_t>(hostChunk_cu, block_size_in_bytes, h_compressSize.data(), out, in,
                                      h_buf_out.data(), outIdx, inIdx);

        // Buffer deleted
        delete (buffer_input);
        delete (buffer_output);
        delete (buffer_compressed_size);
        delete (buffer_block_size);
    }
    
    return outIdx;
}

uint64_t lz4CompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size) {
    std::vector<uint8_t, aligned_allocator<uint8_t>> h_buf_in(BLOCK_SIZE_IN_KB * 1024);
    std::vector<uint8_t, aligned_allocator<uint8_t>> h_buf_out(BLOCK_SIZE_IN_KB * 1024);

    std::vector<uint32_t, aligned_allocator<uint32_t>> h_compressSize(1);

    uint32_t host_buffer_size = BLOCK_SIZE_IN_KB * 1024;
    uint32_t total_block_count = (input_size - 1) / host_buffer_size + 1;

    uint64_t outIdx = 0;
    cl::Buffer *buffer_compressed_size =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), h_compressSize.data());
    // device buffer allocation
    cl::Buffer *buffer_input =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, host_buffer_size, h_buf_in.data());

    cl::Buffer *buffer_output =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, host_buffer_size, h_buf_out.data());

    cl::CommandQueue *m_q= new cl::CommandQueue(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
    cl::Kernel *compress_data_mover_kernel = new cl::Kernel(Application::getInstance().getProgram(), "xilCompDatamover:{xilCompDatamover_1}");
    cl::Kernel *compress_kernel_lz4 = new cl::Kernel(Application::getInstance().getProgram(), "xilLz4CompressStream:{xilLz4CompressStream_1}");

    // sequentially copy block sized buffers to kernel and wait for them to finish before enqueueing
    for (uint32_t blkIndx = 0, bufIndx = 0; blkIndx < total_block_count; blkIndx++, bufIndx += host_buffer_size) {
        // current block input size
        uint32_t c_input_size = host_buffer_size;
        if (blkIndx == total_block_count - 1) c_input_size = input_size - bufIndx;

        // copy input to input buffer
        std::memcpy(h_buf_in.data(), in + bufIndx, c_input_size);

        // set kernel args
        uint32_t narg = 0;
        compress_data_mover_kernel->setArg(narg++, *buffer_input);
        compress_data_mover_kernel->setArg(narg++, *buffer_output);
        compress_data_mover_kernel->setArg(narg++, *buffer_compressed_size);
        compress_data_mover_kernel->setArg(narg, c_input_size);

        compress_kernel_lz4->setArg(2, c_input_size);
        // Migrate Memory - Map host to device buffers
        m_q->enqueueMigrateMemObjects({*(buffer_input)}, 0);
        m_q->finish();

        // enqueue the kernels and wait for them to finish
        m_q->enqueueTask(*compress_data_mover_kernel);
        m_q->enqueueTask(*compress_kernel_lz4);
        m_q->finish();

        // Setup output buffer vectors
        std::vector<cl::Memory> outBufVec;
        outBufVec.push_back(*buffer_output);
        outBufVec.push_back(*buffer_compressed_size);

        // Migrate memory - Map device to host buffers
        m_q->enqueueMigrateMemObjects(outBufVec, CL_MIGRATE_MEM_OBJECT_HOST);
        m_q->finish();
        // read the data to output buffer
        // copy the compressed data to out pointer
        uint32_t compressedSize = h_compressSize.data()[0];

        if (c_input_size > compressedSize) {
            // copy the compressed data
            std::memcpy(out + outIdx, &compressedSize, 4);
            outIdx += 4;
            std::memcpy(out + outIdx, h_buf_out.data(), compressedSize);
            outIdx += compressedSize;
        } else {
            // copy the original data, since no compression
            if (c_input_size == host_buffer_size) {
                out[outIdx++] = 0;
                out[outIdx++] = 0;
                out[outIdx++] = get_bsize(c_input_size);
                out[outIdx++] = xf::compression::NO_COMPRESS_BIT;
            } else {
                uint8_t tmp = c_input_size;
                out[outIdx++] = tmp;
                tmp = c_input_size >> 8;
                out[outIdx++] = tmp;
                tmp = c_input_size >> 16;
                out[outIdx++] = tmp;
                out[outIdx++] = xf::compression::NO_COMPRESS_BIT;
            }
            std::memcpy(out + outIdx, in + (host_buffer_size * blkIndx), c_input_size);
            outIdx += c_input_size;
        }
    }

    // Free CL buffers
    delete (buffer_input);
    delete (buffer_output);
    delete (buffer_compressed_size);

    return outIdx;

}

uint64_t lz4DecompressEngineMM(uint8_t* in, uint8_t* out, size_t input_size, size_t maxOutputSize) {
    size_t host_buffer_size = maxOutputSize;
    uint32_t max_num_blks = (host_buffer_size) / (BLOCK_SIZE_IN_KB * 1024);
    std::vector<uint8_t, aligned_allocator<uint8_t>> h_buf_in(host_buffer_size);
    std::vector<uint8_t, aligned_allocator<uint8_t>> h_buf_out(host_buffer_size);
    std::vector<uint32_t, aligned_allocator<uint32_t>> h_decSize(host_buffer_size);
    std::vector<uint32_t, aligned_allocator<uint32_t>> h_compressSize(host_buffer_size);

    std::vector<uint32_t> m_compressSize(host_buffer_size);

    // Maximum allowed outbuffer size, if it exceeds then exit
    uint32_t c_max_outbuf = host_buffer_size;
    uint32_t block_size_in_bytes = BLOCK_SIZE_IN_KB * 1024;
    uint32_t block_cntr = 0;
    uint32_t done_block_cntr = 0;

    std::chrono::duration<double, std::nano> kernel_time_ns_1(0);
    uint64_t inIdx = 0;
    uint64_t total_decomression_size = 0;

    uint64_t hostChunk_cu;
    uint32_t compute_cu;
    uint64_t output_idx = 0;
    // To handle uncompressed blocks
    bool compressBlk = false;
    // Device buffer allocation
    cl::Buffer *buffer_input =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, host_buffer_size, h_buf_in.data());

    cl::Buffer *buffer_output = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, c_max_outbuf, h_buf_out.data());

    cl::Buffer *buffer_dec_size = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
                                     sizeof(uint32_t) * host_buffer_size, h_decSize.data());

    cl::Buffer *buffer_compressed_size = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                                            sizeof(uint32_t) * host_buffer_size, h_compressSize.data());

    cl::CommandQueue *m_q= new cl::CommandQueue(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
    cl::Kernel *decompress_kernel_lz4 = new cl::Kernel(Application::getInstance().getProgram(), "xilLz4DecompressMM:{xilLz4Decompress_1MM}");

    for (; inIdx < input_size;) {
        compute_cu = 0;
        uint64_t chunk_size = host_buffer_size;

        // Figure out the chunk size for each compute unit
        hostChunk_cu = 0;
        if (inIdx + (chunk_size) > input_size) {
            hostChunk_cu = input_size - (inIdx);
            compute_cu++;
        } else {
            hostChunk_cu = chunk_size;
            compute_cu++;
        }

        uint32_t nblocks;
        uint32_t bufblocks;
        uint64_t total_size;
        uint64_t buf_size;
        uint32_t block_size = 0;
        uint32_t compressed_size = 0;

        nblocks = 0;
        buf_size = 0;
        bufblocks = 0;
        total_size = 0;
        for (uint64_t cIdx = 0; cIdx < hostChunk_cu; nblocks++, total_size += block_size) {
            block_size = block_size_in_bytes;
            std::memcpy(&compressed_size, &in[inIdx], 4);
            inIdx += 4;
            cIdx += 4;

            uint32_t tmp = compressed_size;
            tmp >>= 24;

            if (tmp == 128) {
                uint8_t b1 = compressed_size;
                uint8_t b2 = compressed_size >> 8;
                uint8_t b3 = compressed_size >> 16;
                // uint8_t b4 = compressed_size >> 24;

                if (b3 == 1) {
                    compressed_size = block_size_in_bytes;
                } else {
                    uint16_t size = 0;
                    size = b2;
                    size <<= 8;
                    uint16_t temp = b1;
                    size |= temp;
                    compressed_size = size;
                }
            }
            // Fill original block size and compressed size
            m_compressSize.data()[nblocks] = compressed_size;
            h_compressSize.data()[bufblocks] = compressed_size;
            std::memcpy(&(h_buf_in.data()[buf_size]), &in[inIdx], compressed_size);
            inIdx += compressed_size;
            cIdx += compressed_size;
            buf_size += block_size;
            bufblocks++;
            compressBlk = true;
            block_cntr++;
            done_block_cntr++;
        }

        if (nblocks == 1 && compressed_size == block_size) break;
        if (compressBlk) {
            // Set kernel arguments
            uint32_t narg = 0;
            decompress_kernel_lz4->setArg(narg++, *(buffer_input));
            decompress_kernel_lz4->setArg(narg++, *(buffer_output));
            decompress_kernel_lz4->setArg(narg++, *(buffer_dec_size));
            decompress_kernel_lz4->setArg(narg++, *(buffer_compressed_size));
            decompress_kernel_lz4->setArg(narg++, (uint32_t)BLOCK_SIZE_IN_KB);
            decompress_kernel_lz4->setArg(narg++, bufblocks);

            std::vector<cl::Memory> inBufVec;
            inBufVec.push_back(*(buffer_input));
            inBufVec.push_back(*(buffer_compressed_size));
            // Migrate memory - Map host to device buffers
            m_q->enqueueMigrateMemObjects(inBufVec, 0 /* 0 means from host*/);
            m_q->finish();

            // Kernel invocation
            m_q->enqueueTask(*decompress_kernel_lz4);
            m_q->finish();

            std::vector<cl::Memory> outBufVec;
            outBufVec.push_back(*(buffer_dec_size));
            outBufVec.push_back(*(buffer_output));
            // Migrate memory - Map device to host buffers
            m_q->enqueueMigrateMemObjects(outBufVec, CL_MIGRATE_MEM_OBJECT_HOST);
            m_q->finish();
        }
        uint32_t bufIdx = 0;
        for (uint32_t bIdx = 0; bIdx < nblocks; bIdx++) {
            uint32_t block_size = h_decSize.data()[bIdx];
            if ((output_idx + block_size) > c_max_outbuf) {
                std::cout << "\n" << std::endl;
                std::cout << "\x1B[35mZIP BOMB: Exceeded output buffer size during decompression \033[0m \n"
                          << std::endl;
                std::cout
                    << "\x1B[35mUse -mcr option to increase the maximum compression ratio (Default: 10) \033[0m \n"
                    << std::endl;
                std::cout << "\x1B[35mAborting .... \033[0m\n" << std::endl;
                exit(1);
            }
            std::memcpy(&out[output_idx], &h_buf_out.data()[bufIdx], block_size);
            output_idx += block_size;
            bufIdx += block_size;
            total_decomression_size += block_size;
        }

    } // Top - Main loop ends here
    // Delete device buffers
    delete (buffer_input);
    delete (buffer_output);
    delete (buffer_dec_size);
    delete (buffer_compressed_size);
    h_buf_in.clear();
    h_buf_out.clear();

    return total_decomression_size;
}

uint64_t lz4DecompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size, size_t maxOutputSize) {
    std::vector<uint32_t, aligned_allocator<uint32_t> > decompressSize;
    uint32_t outputSize = maxOutputSize;
    
    // Index calculation
    std::vector<uint8_t, aligned_allocator<uint8_t>> h_buf_in(input_size);
    std::vector<uint8_t, aligned_allocator<uint8_t>> h_buf_out(outputSize);
    std::vector<uint32_t, aligned_allocator<uint32_t>> h_buf_decompressSize(sizeof(uint32_t));

    std::memcpy(h_buf_in.data(), in, input_size);

    // Device buffer allocation
    cl::Buffer *buffer_input = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, input_size, h_buf_in.data());
    cl::Buffer *buffer_output = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, outputSize, h_buf_out.data());
    cl::Buffer *bufferOutputSize = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t),
                                      h_buf_decompressSize.data());

    uint32_t inputSize_32t = uint32_t(input_size);

    // set kernel arguments
    cl::Kernel *decompress_data_mover_kernel = new cl::Kernel(Application::getInstance().getProgram(), "xilDecompDatamover:{xilDecompDatamover_1}");
    int narg = 0;
    decompress_data_mover_kernel->setArg(narg++, *(buffer_input));
    decompress_data_mover_kernel->setArg(narg++, *(buffer_output));
    decompress_data_mover_kernel->setArg(narg++, inputSize_32t);
    decompress_data_mover_kernel->setArg(narg, *(bufferOutputSize));

    cl::Kernel *decompress_kernel_lz4 = new cl::Kernel(Application::getInstance().getProgram(), "xilLz4CompressStream:{xilLz4CompressStream_1}");
    decompress_kernel_lz4->setArg(3, inputSize_32t);

    // Migrate Memory - Map host to device buffers
    cl::CommandQueue *m_q= new cl::CommandQueue(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);
    m_q->enqueueMigrateMemObjects({*(buffer_input), *(bufferOutputSize), *(buffer_output)}, 0);
    m_q->finish();

    // enqueue the kernels and wait for them to finish
    m_q->enqueueTask(*decompress_data_mover_kernel);
    m_q->enqueueTask(*decompress_kernel_lz4);
    m_q->finish();

    // Migrate memory - Map device to host buffers
    m_q->enqueueMigrateMemObjects({*(buffer_output), *(bufferOutputSize)}, CL_MIGRATE_MEM_OBJECT_HOST);
    m_q->finish();

    uint32_t uncompressedSize = h_buf_decompressSize[0];
    std::memcpy(out, h_buf_out.data(), uncompressedSize);

    delete (buffer_input);
    buffer_input = nullptr;
    delete (buffer_output);
    buffer_output = nullptr;
    h_buf_in.clear();
    h_buf_out.clear();

    return uncompressedSize;
}

uint64_t lz4CompressMM(uint8_t* in, uint8_t* out, size_t input_size) {
    uint64_t outIdx=0;

    // LZ4 header
    outIdx += writeLz4Header(out, input_size);

    uint64_t enbytes = lz4CompressEngineMM(in, out, input_size);
    outIdx+=enbytes;

    outIdx+=writeLz4Footer(in, out, input_size);
    return outIdx;
}

uint64_t lz4DecompressMM(uint8_t* in, uint8_t* out, size_t input_size, size_t maxOutputSize) {
    in += readLz4Header(in);
    input_size = input_size - 15;

    uint64_t debytes = lz4DecompressEngineMM(in, out, input_size, maxOutputSize);

    return debytes;
}

uint64_t lz4DecompressStream(uint8_t* in, uint8_t* out, size_t input_size, size_t maxOutputSize) {

    uint64_t debytes = lz4DecompressEngineStream(in, out, input_size, maxOutputSize);

    return debytes;
}

// uint64_t lz4CompressMM(uint8_t* in, uint8_t* out, size_t input_size) {
//     uint64_t outIdx=0;

//     // LZ4 header
//     outIdx += writeLz4Header(out, input_size);

//     uint64_t enbytes;

//     if (m_lz4Stream) {
//         enbytes = compressEngineStreamSeq(in, out, m_InputSize);
//     } else {
//         enbytes = compressEngineSeq(in, out, m_InputSize);
//     }

//     // lz4 frame formatting
//     out = out + enbytes;
//     writeFooter(in, out);
//     enbytes += m_frameByteCount;
//     return enbytes;
// }
}
}