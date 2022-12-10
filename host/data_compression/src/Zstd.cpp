#include "Zstd.hpp"

namespace dataCompression{
namespace internal{
uint64_t zstdCompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size){
    cl_int err;
    auto c_inputSize = input_size;

    // host allocated aligned memory
    std::vector<uint8_t, aligned_allocator<uint8_t>> cbuf_in(c_inputSize);
    std::vector<uint8_t, aligned_allocator<uint8_t>> cbuf_out(c_inputSize);
    std::vector<uint64_t, aligned_allocator<uint64_t>> cbuf_outSize(2);

    BufferPointer buffer_cmp_input(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, c_inputSize, cbuf_in.data());
    BufferPointer buffer_cmp_output(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, c_inputSize, cbuf_out.data());
    BufferPointer buffer_cmp_size(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), cbuf_outSize.data());

    // set consistent kernel arguments
    KernelPointer cmp_dm_kernel(Application::getProgram<Lib::ZSTD>(), "xilZstdCompressDataMover:{xilZstdCompressDataMover_1}");
    cmp_dm_kernel->setArg(0, *buffer_cmp_input);
    cmp_dm_kernel->setArg(1, *buffer_cmp_output);
    cmp_dm_kernel->setArg(3, *buffer_cmp_size);

    CommandQueuePointer m_q_cdm(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);

    auto enbytes = 0;
    auto outIdx = 0;
    // Process the data serially
    for (size_t inIdx = 0; inIdx < input_size; inIdx += c_inputSize) {
        std::cout<<"test"<<std::endl;
        uint32_t buf_size = c_inputSize;
        if (inIdx + buf_size > input_size) buf_size = input_size - inIdx;

        // Copy input data
        std::memcpy(cbuf_in.data(), &in[inIdx], buf_size);

        // Set Variable Kernel Args
        cmp_dm_kernel->setArg(2, buf_size);

        // Transfer the data to device
        OCL_CHECK(err, err = m_q_cdm->enqueueMigrateMemObjects({*(buffer_cmp_input)}, 0, nullptr, nullptr));
        m_q_cdm->finish();

        // printf("Free running kernel\n");
        m_q_cdm->enqueueTask(*cmp_dm_kernel);
        m_q_cdm->finish();

        // Transfer the data from device to host
        OCL_CHECK(err, err = m_q_cdm->enqueueMigrateMemObjects({*(buffer_cmp_output), *(buffer_cmp_size)},
                                                               CL_MIGRATE_MEM_OBJECT_HOST));
        m_q_cdm->finish();

        auto compSize = cbuf_outSize[0];
        std::memcpy(out + outIdx, cbuf_out.data(), compSize);
        enbytes += compSize;
        outIdx += compSize;
    }
    m_q_cdm->finish();

    return enbytes;
}

uint64_t zstdDecompressEngineStream(uint8_t* in, uint8_t* out, size_t input_size, uint64_t max_outbuf_size){
    std::chrono::duration<double, std::nano> kernel_time_ns_1(0);
    uint32_t inBufferSize = input_size;
    uint32_t isLast = 1;
    const uint32_t lim_4gb = (uint32_t)(((uint64_t)4 * 1024 * 1024 * 1024) - 2); // 4GB limit on output size
    uint32_t outBufferSize = 0;
    // allocate < 4GB size for output buffer
    if (max_outbuf_size > lim_4gb) {
        outBufferSize = lim_4gb;
    } else {
        outBufferSize = (uint32_t)max_outbuf_size;
    }
    // host allocated aligned memory
    std::vector<uint8_t, aligned_allocator<uint8_t>> dbuf_in(inBufferSize);
    std::vector<uint8_t, aligned_allocator<uint8_t>> dbuf_out(outBufferSize);
    std::vector<uint64_t, aligned_allocator<uint64_t>> dbuf_outSize(2);
    std::vector<uint32_t, aligned_allocator<uint32_t>> h_dcompressStatus(2);

    BufferPointer buffer_dec_input(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, inBufferSize, dbuf_in.data());
    BufferPointer buffer_dec_output(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, outBufferSize, dbuf_out.data());
    BufferPointer buffer_size(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), dbuf_outSize.data());
    BufferPointer buffer_status(Application::getContext<Lib::ZSTD>(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t), h_dcompressStatus.data());

    // Set Kernel Args
    KernelPointer data_writer_kernel(Application::getProgram<Lib::ZSTD>(), "xilMM2S:{xilMM2S_2}");
    KernelPointer data_reader_kernel(Application::getProgram<Lib::ZSTD>(), "xilS2MM:{xilS2MM_2}");

    data_writer_kernel->setArg(0, *(buffer_dec_input));
    data_writer_kernel->setArg(1, inBufferSize);
    data_writer_kernel->setArg(2, isLast);

    data_reader_kernel->setArg(0, *(buffer_dec_output));
    data_reader_kernel->setArg(1, *(buffer_size));
    data_reader_kernel->setArg(2, *(buffer_status));
    data_reader_kernel->setArg(3, outBufferSize);

    CommandQueuePointer m_q_rd(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);
    CommandQueuePointer m_q_wr(Application::getContext<Lib::ZSTD>(), Application::getDevice<Lib::ZSTD>(), CL_QUEUE_PROFILING_ENABLE);

    // Copy input data
    std::memcpy(dbuf_in.data(), in, inBufferSize); // must be equal to input_size
    m_q_wr->enqueueMigrateMemObjects({*(buffer_dec_input)}, 0, NULL, NULL);
    m_q_wr->finish();

    // start parallel reader kernel enqueue thread
    uint64_t decmpSizeIdx = 0;

    // enqueue data movers
    m_q_rd->enqueueTask(*data_reader_kernel);

    m_q_wr->enqueueTask(*data_writer_kernel);
    m_q_wr->finish();

    // copy decompressed output data
    m_q_rd->enqueueMigrateMemObjects({*(buffer_size), *(buffer_dec_output)}, CL_MIGRATE_MEM_OBJECT_HOST, NULL, NULL);
    m_q_rd->finish();
    // decompressed size
    decmpSizeIdx = dbuf_outSize[0];
    // copy output decompressed data
    std::memcpy(out, dbuf_out.data(), decmpSizeIdx);

    return decmpSizeIdx;
}

uint64_t zstdCompressStream(uint8_t* in, uint8_t* out, size_t input_size){
    uint64_t enbytes = zstdCompressEngineStream(in, out, input_size);
    return enbytes;
}

uint64_t zstdDecompressStream(uint8_t* in, uint8_t* out, size_t input_size, uint64_t maxOutputSize){
    uint64_t debytes = zstdDecompressEngineStream(in, out, input_size, maxOutputSize);
    return debytes;
}
}
}