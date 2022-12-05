#include "Snappy.hpp"

namespace dataCompression{
namespace internal{
uint8_t writeHeader(uint8_t* out){
    int fileIdx = 0;

    // Snappy Stream Identifier
    out[fileIdx++] = 0xff;
    out[fileIdx++] = 0x06;
    out[fileIdx++] = 0x00;
    out[fileIdx++] = 0x00;
    out[fileIdx++] = 0x73;
    out[fileIdx++] = 0x4e;
    out[fileIdx++] = 0x61;
    out[fileIdx++] = 0x50;
    out[fileIdx++] = 0x70;
    out[fileIdx++] = 0x59;

    return fileIdx;
}

uint8_t readHeader(uint8_t* in){
    uint8_t fileIdx = 0;
    uint8_t header_byte = 10;
    fileIdx = header_byte;
    return fileIdx;
}

uint64_t snappyCompressEngineMM(uint8_t* in, uint8_t* out, uint64_t input_size){
    KernelPointer compress_kernel_snappy;
    CommandQueuePointer m_q;
    BufferPointer buffer_input, buffer_output, buffer_compressed_size, buffer_block_size;
    compress_kernel_snappy.create(Application::getInstance().getProgram(), "xilSnappyCompressMM:{xilSnappyCompressMM_1}");
    m_q.create(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    std::vector<uint8_t, aligned_allocator<uint8_t> > h_buf_in(HOST_BUFFER_SIZE);
    std::vector<uint8_t, aligned_allocator<uint8_t> > h_buf_out(HOST_BUFFER_SIZE);
    std::vector<uint32_t, aligned_allocator<uint32_t> > h_compressSize(MAX_NUMBER_BLOCKS);
    std::vector<uint32_t, aligned_allocator<uint32_t> > h_blksize(MAX_NUMBER_BLOCKS);

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
    // This buffer contains total number of BLOCK_SIZE_IN_KB blocks per CU
    // For Example: HOST_BUFFER_SIZE = 2MB/BLOCK_SIZE_IN_KB = 32block (Block
    // size 64 by default)
    uint32_t total_blocks_cu;

    // This buffer holds exact size of the chunk in bytes for all the CUs
    uint32_t bufSize_in_bytes_cu;

    // Holds value of total compute units to be
    // used per iteration
    int compute_cu = 0;
    for (uint64_t inIdx = 0; inIdx < input_size; inIdx += HOST_BUFFER_SIZE) {
        // Needs to reset this variable
        // As this drives compute unit launch per iteration
        compute_cu = 0;

        // Pick buffer size as predefined one
        // If yet to be consumed input is lesser
        // the reset to required size

        uint32_t buf_size = HOST_BUFFER_SIZE;

        // This loop traverses through each compute based current inIdx
        // It tries to calculate chunk size and total compute units need to be
        // launched (based on the input_size)

        for (int bufCalc = 0; bufCalc < 1; bufCalc++) {
            hostChunk_cu = 0;
            // If amount of data to be consumed is less than HOST_BUFFER_SIZE
            // Then choose to send is what is needed instead of full buffer size
            // based on host buffer macro

            if (inIdx + (buf_size * (bufCalc + 1)) > input_size) {
                hostChunk_cu = input_size - (inIdx + HOST_BUFFER_SIZE * bufCalc);
                compute_cu++;
                break;
            } else {
                hostChunk_cu = buf_size;
                compute_cu++;
            }
        }
        // Figure out total number of blocks need per each chunk
        // Copy input data from in to host buffer based on the inIdx and cu

        for (int blkCalc = 0; blkCalc < compute_cu; blkCalc++) {
            uint32_t nblocks = (hostChunk_cu - 1) / block_size_in_bytes + 1;
            total_blocks_cu = nblocks;
            std::memcpy(h_buf_in.data(), &in[inIdx + blkCalc * HOST_BUFFER_SIZE], hostChunk_cu);
        }
        // Fill the host block size buffer with various block sizes per chunk/cu
        for (int cuBsize = 0; cuBsize < compute_cu; cuBsize++) {
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
        }

        buffer_input.create(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, bufSize_in_bytes_cu, h_buf_in.data());
        buffer_output.create(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, bufSize_in_bytes_cu, h_buf_out.data());
        buffer_compressed_size.create(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t) * total_blocks_cu, h_compressSize.data());
        buffer_block_size.create(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t) * total_blocks_cu, h_blksize.data());

        int narg = 0;
        compress_kernel_snappy->setArg(narg++, *(buffer_input));
        compress_kernel_snappy->setArg(narg++, *(buffer_output));
        compress_kernel_snappy->setArg(narg++, *(buffer_compressed_size));
        compress_kernel_snappy->setArg(narg++, *(buffer_block_size));
        compress_kernel_snappy->setArg(narg++, block_size_in_kb);
        compress_kernel_snappy->setArg(narg++, hostChunk_cu);
        std::vector<cl::Memory> inBufVec;

        inBufVec.push_back(*(buffer_input));
        inBufVec.push_back(*(buffer_block_size));

        // Migrate memory - Map host to device buffers
        m_q->enqueueMigrateMemObjects(inBufVec, 0 /* 0 means from host*/);
        m_q->finish();

        // Fire kernel execution
        m_q->enqueueTask(*compress_kernel_snappy);
        // wait till kernels complete
        m_q->finish();

        // Migrate memory - Map device to host buffers
        m_q->enqueueMigrateMemObjects({*(buffer_output), *(buffer_compressed_size)}, CL_MIGRATE_MEM_OBJECT_HOST);
        m_q->finish();
        for (int cuCopy = 0; cuCopy < compute_cu; cuCopy++) {
            // Copy data into out buffer
            // Include compress and block size data
            // Copy data block by block within a chunk example 2MB (64block size) - 32 blocks data
            // Do the same for all the compute units
            uint32_t idx = 0;
            for (uint32_t bIdx = 0; bIdx < total_blocks_cu; bIdx++, idx += block_size_in_bytes) {
                // Default block size in bytes i.e., 64 * 1024
                uint32_t block_size = block_size_in_bytes;
                if (idx + block_size > hostChunk_cu) {
                    block_size = hostChunk_cu - idx;
                }
                uint32_t compressed_size = h_compressSize.data()[bIdx];
                assert(compressed_size != 0);

                int orig_block_size = hostChunk_cu;
                int perc_cal = orig_block_size * 10;
                perc_cal = perc_cal / block_size;
                if (compressed_size < block_size && perc_cal >= 10) {
                    // Chunk Type Identifier
                    out[outIdx++] = 0x00;
                    // 3 Bytes to represent compress block length + 4;
                    uint32_t f_csize = compressed_size + 4;
                    std::memcpy(&out[outIdx], &f_csize, 3);
                    outIdx += 3;
                    // CRC - for now 0s
                    uint32_t crc_value = 0;
                    std::memcpy(&out[outIdx], &crc_value, 4);
                    outIdx += 4;
                    // Compressed data of this block with preamble
                    std::memcpy(&out[outIdx], (h_buf_out.data() + bIdx * block_size_in_bytes), compressed_size);
                    outIdx += compressed_size;
                } else {
                    // Chunk Type Identifier
                    out[outIdx++] = 0x01;
                    // 3 Bytes to represent uncompress block length + 4;
                    uint32_t f_csize = block_size + 4;
                    std::memcpy(&out[outIdx], &f_csize, 3);
                    outIdx += 3;
                    // CRC -for now 0s
                    uint32_t crc_value = 0;
                    std::memcpy(&out[outIdx], &crc_value, 4);
                    outIdx += 4;

                    // Uncompressed data copy
                    std::memcpy(&out[outIdx], &in[inIdx + (cuCopy * HOST_BUFFER_SIZE) + idx], block_size);
                    outIdx += block_size;
                } // End of else - uncompressed stream update
            }     // End of chunk (block by block) copy to output buffer
        }         // End of CU loop - Each CU/chunk block by block copy
                  // Buffer deleted
    }
    return outIdx;
}

uint64_t snappyCompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size){
    cl::Kernel *compress_kernel_snappy = new cl::Kernel(Application::getInstance().getProgram(), "xilSnappyCompressStream:{xilSnappyCompressStream_1}");
    cl::Kernel *compress_data_mover_kernel = new cl::Kernel(Application::getInstance().getProgram(), "xilCompressDatamover:{xilCompressDatamover_1}");
    cl::CommandQueue *m_q = new cl::CommandQueue(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    uint32_t host_buffer_size = BLOCK_SIZE_IN_KB * 1024;
    uint32_t total_block_count = (input_size - 1) / host_buffer_size + 1;

    // output buffer index
    uint64_t outIdx = 0;
    // Index calculation
    std::vector<uint8_t, aligned_allocator<uint8_t> > h_buf_in(host_buffer_size);
    std::vector<uint8_t, aligned_allocator<uint8_t> > h_buf_out(host_buffer_size);
    std::vector<uint32_t, aligned_allocator<uint32_t> > h_compressSize(1);
    // h_buf_in.resize(host_buffer_size);  // * total_block_count);
    // h_buf_out.resize(host_buffer_size); // * total_block_count);
    // h_compressSize.resize(1);

    // device buffer allocation
    cl::Buffer *buffer_input =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, host_buffer_size, h_buf_in.data());

    cl::Buffer *buffer_output =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, host_buffer_size, h_buf_out.data());

    cl::Buffer *buffer_compressed_size =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), h_compressSize.data());

    // copy input to input buffer
    // std::memcpy(h_buf_in.data(), in, input_size);
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

        compress_kernel_snappy->setArg(2, c_input_size);
        // Migrate Memory - Map host to device buffers
        m_q->enqueueMigrateMemObjects({*(buffer_input)}, 0);
        m_q->finish();
        
        // enqueue the kernels and wait for them to finish
        m_q->enqueueTask(*compress_data_mover_kernel);
        m_q->enqueueTask(*compress_kernel_snappy);
        m_q->finish();

        // Setup output buffer vectors
        std::vector<cl::Memory> outBufVec;
        outBufVec.push_back(*buffer_output);
        outBufVec.push_back(*buffer_compressed_size);

        // Migrate memory - Map device to host buffers
        m_q->enqueueMigrateMemObjects(outBufVec, CL_MIGRATE_MEM_OBJECT_HOST);
        m_q->finish();

        // copy the compressed data to out pointer
        uint32_t compressedSize = h_compressSize.data()[0];

        if (c_input_size > compressedSize) {
            out[outIdx++] = 0x00;

            // 3 Bytes to represent compressed block length + 4
            uint32_t f_csize = compressedSize + 4;
            std::memcpy(out + outIdx, &f_csize, 3);
            outIdx += 3;

            // CRC - for now 0s
            uint32_t crc_value = 0;
            std::memcpy(out + outIdx, &crc_value, 4);
            outIdx += 4;

            std::memcpy(out + outIdx, h_buf_out.data(), compressedSize);
            outIdx += compressedSize;
        } else {
            // Chunk Type Identifier
            out[outIdx++] = 0x01;
            // 3 Bytes to represent uncompress block length + 4;
            uint32_t f_csize = c_input_size + 4;
            std::memcpy(out + outIdx, &f_csize, 3);
            outIdx += 3;
            // CRC -for now 0s
            uint32_t crc_value = 0;
            std::memcpy(out + outIdx, &crc_value, 4);
            outIdx += 4;

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

uint64_t snappyDecompressEngineMM(uint8_t* in, uint8_t* out, size_t input_size){
    KernelPointer decompress_kernel_snappy;
    CommandQueuePointer m_q;
    BufferPointer buffer_input, buffer_output, buffer_block_size, buffer_compressed_size;
    decompress_kernel_snappy.create(Application::getInstance().getProgram(), "xilSnappyDecompressMM:{xilSnappyDecompressMM_1}");
    m_q.create(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    std::vector<uint8_t, aligned_allocator<uint8_t> > h_buf_in(HOST_BUFFER_SIZE);
    std::vector<uint8_t, aligned_allocator<uint8_t> > h_buf_out(HOST_BUFFER_SIZE);
    std::vector<uint32_t, aligned_allocator<uint32_t> > h_compressSize(MAX_NUMBER_BLOCKS);
    std::vector<uint32_t, aligned_allocator<uint32_t> > h_blksize(MAX_NUMBER_BLOCKS);
    std::vector<uint32_t> m_blkSize(MAX_NUMBER_BLOCKS);
    std::vector<uint32_t> m_compressSize(MAX_NUMBER_BLOCKS);

    uint32_t buf_size = BLOCK_SIZE_IN_KB * 1024;
    uint32_t blocksPerChunk = HOST_BUFFER_SIZE / buf_size;
    uint32_t host_buffer_size = ((HOST_BUFFER_SIZE - 1) / BLOCK_SIZE_IN_KB + 1) * BLOCK_SIZE_IN_KB;

    buffer_input.create(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, host_buffer_size, h_buf_in.data());
    buffer_output.create(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, host_buffer_size, h_buf_out.data());
    buffer_block_size.create(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t) * blocksPerChunk, h_blksize.data());
    buffer_compressed_size.create(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t) * blocksPerChunk, h_compressSize.data());
                                        
    uint32_t narg = 0;
    decompress_kernel_snappy->setArg(narg++, *(buffer_input));
    decompress_kernel_snappy->setArg(narg++, *(buffer_output));
    decompress_kernel_snappy->setArg(narg++, *(buffer_block_size));
    decompress_kernel_snappy->setArg(narg++, *(buffer_compressed_size));
    decompress_kernel_snappy->setArg(narg++, (uint32_t)BLOCK_SIZE_IN_KB);
    decompress_kernel_snappy->setArg(narg++, blocksPerChunk);
    uint32_t chunk_size = 0;
    uint8_t chunk_idx = 0;
    uint32_t block_cntr = 0;
    uint32_t block_size = 0;
    uint32_t chunk_cntr = 0;
    uint32_t bufblocks = 0;
    uint64_t output_idx = 0;
    uint32_t bufIdx = 0;
    uint32_t over_block_cntr = 0;
    uint32_t brick = 0;
    uint16_t stride_cidsize = 4;
    bool blkDecomExist = false;
    uint32_t blkUnComp = 0;
    // Maximum allowed outbuffer size, if it exceeds then exit
    uint32_t c_max_outbuf = input_size * 20; //(20=m_maxCR by default)
    // Go over overall input size
    for (uint32_t idxSize = 0; idxSize < input_size; idxSize += stride_cidsize, chunk_cntr++) {
        // Chunk identifier
        chunk_idx = in[idxSize];
        chunk_size = 0;
        // Chunk Compressed size
        uint8_t cbyte_1 = in[idxSize + 1];
        uint8_t cbyte_2 = in[idxSize + 2];
        uint8_t cbyte_3 = in[idxSize + 3];

        uint32_t temp = cbyte_3;
        temp <<= 16;
        chunk_size |= temp;
        temp = 0;
        temp = cbyte_2;
        temp <<= 8;
        chunk_size |= temp;
        temp = 0;
        chunk_size |= cbyte_1;

        if (chunk_idx == 0x00) {
            uint8_t bval1 = in[idxSize + 8];
            uint32_t final_size = 0;

            if ((bval1 >> 7) == 1) {
                uint8_t b1 = bval1 & 0x7F;
                bval1 = in[idxSize + 9];
                uint8_t b2 = bval1 & 0x7F;
                if ((bval1 >> 7) == 1) {
                    bval1 = in[idxSize + 10];
                    uint8_t b3 = bval1 & 0x7F;
                    uint32_t temp1 = b3;
                    temp1 <<= 14;
                    uint32_t temp2 = b2;
                    temp2 <<= 7;
                    uint32_t temp3 = b1;
                    final_size |= temp1;
                    final_size |= temp2;
                    final_size |= temp3;
                } else {
                    uint32_t temp1 = b2;
                    temp1 <<= 7;
                    uint32_t temp2 = b1;
                    final_size |= temp1;
                    final_size |= temp2;
                }

                block_size = final_size;
            } else {
                block_size = bval1;
            }
            m_compressSize.data()[over_block_cntr] = chunk_size - 4;
            m_blkSize.data()[over_block_cntr] = block_size;

            h_compressSize.data()[bufblocks] = chunk_size - 4;
            h_blksize.data()[bufblocks] = block_size;
            bufblocks++;
            // Copy data
            std::memcpy(&(h_buf_in.data()[block_cntr * buf_size]), &in[idxSize + 8], chunk_size - 4);
            block_cntr++;
            blkDecomExist = true;
        } else if (chunk_idx == 0x01) {
            m_compressSize.data()[over_block_cntr] = chunk_size - 4;
            m_blkSize.data()[over_block_cntr] = chunk_size - 4;
            std::memcpy(&out[brick * HOST_BUFFER_SIZE + over_block_cntr * buf_size], &in[idxSize + 8], chunk_size - 4);
            blkUnComp += chunk_size - 4;
        }

        over_block_cntr++;
        // Increment the input idx to
        // compressed size length
        idxSize += chunk_size;

        if (over_block_cntr == blocksPerChunk && blkDecomExist) {
            blkDecomExist = false;
            // Track the chunks processed
            brick++;
            // In case of left over set kernel arg to no blocks
            decompress_kernel_snappy->setArg(5, block_cntr);
            // For big files go ahead do it here
            // Migrate memory - Map host to device buffers
            m_q->enqueueMigrateMemObjects({*(buffer_input), *(buffer_block_size), *(buffer_compressed_size)}, 0 /*0 means from host*/);
            m_q->finish();

            m_q->enqueueTask(*decompress_kernel_snappy);
            m_q->finish();

            // Migrate memory - Map device to host buffers
            m_q->enqueueMigrateMemObjects({*(buffer_output)}, CL_MIGRATE_MEM_OBJECT_HOST);
            m_q->finish();
            bufIdx = 0;
            // copy output
            for (uint32_t bIdx = 0; bIdx < over_block_cntr; bIdx++) {
                uint32_t block_size = m_blkSize.data()[bIdx];
                uint32_t compressed_size = m_compressSize.data()[bIdx];

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

                if (compressed_size < block_size) {
                    std::memcpy(&out[output_idx], &h_buf_out.data()[bufIdx], block_size);
                    output_idx += block_size;
                    bufIdx += block_size;
                } else if (compressed_size == block_size) {
                    output_idx += block_size;
                    blkUnComp -= block_size;
                }
            }
            block_cntr = 0;
            bufblocks = 0;
            over_block_cntr = 0;
        } else if (over_block_cntr == blocksPerChunk) {
            over_block_cntr = 0;
            brick++;
            bufblocks = 0;
            block_cntr = 0;
        }
    }

    if (block_cntr != 0) {
        // In case of left over set kernel arg to no blocks
        decompress_kernel_snappy->setArg(5, block_cntr);

        // Migrate memory - Map host to device buffers
        m_q->enqueueMigrateMemObjects({*(buffer_input), *(buffer_block_size), *(buffer_compressed_size)}, 0 /*0 means from host*/);
        m_q->finish();
        
        // Kernel invocation
        m_q->enqueueTask(*decompress_kernel_snappy);
        m_q->finish();

        // Migrate memory - Map device to host buffers
        m_q->enqueueMigrateMemObjects({*(buffer_output)}, CL_MIGRATE_MEM_OBJECT_HOST);
        m_q->finish();
        bufIdx = 0;
        // copy output
        for (uint32_t bIdx = 0; bIdx < over_block_cntr; bIdx++) {
            uint32_t block_size = m_blkSize.data()[bIdx];
            uint32_t compressed_size = m_compressSize.data()[bIdx];

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

            if (compressed_size < block_size) {
                std::memcpy(&out[output_idx], &h_buf_out.data()[bufIdx], block_size);
                output_idx += block_size;
                bufIdx += block_size;
            } else if (compressed_size == block_size) {
                output_idx += block_size;
                blkUnComp -= block_size;
            }
        }

    } // If to see if tehr eare some blocks to be processed
    if (output_idx == 0 && blkUnComp != 0) {
        output_idx = blkUnComp;
    }

    return output_idx;
}

uint64_t snappyDecompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size){
    
    cl::Kernel *decompress_kernel_snappy = new cl::Kernel(Application::getInstance().getProgram(), "xilSnappyDecompressStream:{xilSnappyDecompressStream_1}");
    cl::Kernel *decompress_data_mover_kernel = new cl::Kernel(Application::getInstance().getProgram(), "xilDecompressDatamover:{xilDecompressDatamover_1}");
    cl::CommandQueue *m_q = new cl::CommandQueue(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

    uint32_t outputSize = (input_size * 20) + 16; //m_maxCR
    // cl::Buffer* bufferOutputSize;
    // Index calculation
    // h_buf_in.resize(input_size);
    // h_buf_out.resize(outputSize);
    // h_buf_decompressSize.resize(sizeof(uint32_t));
    std::vector<uint8_t, aligned_allocator<uint8_t> > h_buf_in(input_size);
    std::vector<uint8_t, aligned_allocator<uint8_t> > h_buf_out(outputSize);
    std::vector<uint32_t, aligned_allocator<uint32_t> > h_buf_decompressSize(1);

    std::memcpy(h_buf_in.data(), in, input_size);

    // Device buffer allocation
    cl::Buffer *buffer_input = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, input_size, h_buf_in.data());
    cl::Buffer *buffer_output = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, outputSize, h_buf_out.data());
    cl::Buffer *bufferOutputSize = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, sizeof(uint32_t),
                                      h_buf_decompressSize.data());

    uint32_t inputSize_32t = uint32_t(input_size);
    // set kernel arguments
    int narg = 0;
    decompress_data_mover_kernel->setArg(narg++, *(buffer_input));
    decompress_data_mover_kernel->setArg(narg++, *(buffer_output));
    decompress_data_mover_kernel->setArg(narg++, inputSize_32t);
    decompress_data_mover_kernel->setArg(narg, *(bufferOutputSize));

    decompress_kernel_snappy->setArg(3, inputSize_32t);

    // Migrate Memory - Map host to device buffers
    m_q->enqueueMigrateMemObjects({*(buffer_input), *(bufferOutputSize), *(buffer_output)}, 0);
    m_q->finish();

    // enqueue the kernels and wait for them to finish
    m_q->enqueueTask(*decompress_data_mover_kernel);
    m_q->enqueueTask(*decompress_kernel_snappy);
    m_q->finish();
    
    // Migrate memory - Map device to host buffers
    m_q->enqueueMigrateMemObjects({*(buffer_output), *(bufferOutputSize)}, CL_MIGRATE_MEM_OBJECT_HOST);
    m_q->finish();

    uint32_t uncompressedSize = h_buf_decompressSize[0];
    std::memcpy(out, h_buf_out.data(), uncompressedSize);

    delete buffer_input;
    buffer_input = nullptr;
    delete buffer_output;
    buffer_output = nullptr;
    h_buf_in.clear();
    h_buf_out.clear();

    return uncompressedSize;
}

uint64_t snappyCompressMM(uint8_t* in, uint8_t* out, uint64_t input_size){
    std::cout<<"Before Snappy Compress: "<<std::endl;
    hexdump(in, input_size);

    uint32_t outIdx=writeHeader(out);
    uint64_t enbytes=snappyCompressEngineMM(in, out+outIdx, input_size);
    outIdx+=enbytes;
    std::cout<<"After Snappy Compress: "<<std::endl;
    hexdump(out, outIdx);
    return outIdx;
}

uint64_t snappyCompressStream(uint8_t* in, uint8_t* out, uint64_t input_size){
    std::cout<<"Before Snappy Compress: "<<std::endl;
    hexdump(in, input_size);

    uint32_t outIdx=writeHeader(out);
    uint64_t enbytes=snappyCompressEngineStream(in, out+outIdx, input_size);
    outIdx+=enbytes;
    std::cout<<"After Snappy Compress: "<<std::endl;
    hexdump(out, outIdx);
    return outIdx;
}

uint64_t snappyDecompressMM(uint8_t* in, uint8_t* out, uint64_t input_size){
    std::cout<<"Before Snappy Decompress: "<<std::endl;
    hexdump(in, input_size);

    uint8_t headerBytes = readHeader(in);
    uint64_t debytes = snappyDecompressEngineMM(in+headerBytes, out, input_size-headerBytes);

    std::cout<<"After Snappy Decompress: "<<std::endl;
    hexdump(out, debytes);
    return debytes;
}

uint64_t snappyDecompressStream(uint8_t* in, uint8_t* out, uint64_t input_size){
    std::cout<<"Before Snappy Decompress: "<<std::endl;
    hexdump(in, input_size);

    uint64_t debytes = snappyDecompressEngineStream(in, out, input_size);

    std::cout<<"After Snappy Decompress: "<<std::endl;
    hexdump(out, debytes);
    return debytes;
}
} //internal
} //dataCompression