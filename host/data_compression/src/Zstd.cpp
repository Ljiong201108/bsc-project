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

    // opencl buffer creation
    OCL_CHECK(err, cl::Buffer *buffer_cmp_input = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, c_inputSize,
                                                     cbuf_in.data(), &err));
    OCL_CHECK(err, cl::Buffer *buffer_cmp_output = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, c_inputSize,
                                                      cbuf_out.data(), &err));

    OCL_CHECK(err, cl::Buffer *buffer_cmp_size = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,
                                                    sizeof(uint32_t), cbuf_outSize.data(), &err));

    // set consistent kernel arguments
    cl::Kernel *cmp_dm_kernel = new cl::Kernel(Application::getInstance().getProgram(), "xilZstdCompressDataMover:{xilZstdCompressDataMover_1}");
    cmp_dm_kernel->setArg(0, *buffer_cmp_input);
    cmp_dm_kernel->setArg(1, *buffer_cmp_output);
    cmp_dm_kernel->setArg(3, *buffer_cmp_size);

    cl::CommandQueue *m_q_cdm = new cl::CommandQueue(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_PROFILING_ENABLE);

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

    // free the cl buffers
    delete buffer_cmp_input;
    delete buffer_cmp_output;
    delete buffer_cmp_size;

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

    // opencl buffer creation
    cl::Buffer *buffer_dec_input = new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, inBufferSize, dbuf_in.data());
    cl::Buffer *buffer_dec_output =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, outBufferSize, dbuf_out.data());

    cl::Buffer *buffer_size =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(uint32_t), dbuf_outSize.data());
    cl::Buffer *buffer_status =
        new cl::Buffer(Application::getInstance().getContext(), CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(uint32_t), h_dcompressStatus.data());

    // Set Kernel Args
    cl::Kernel *data_writer_kernel = new cl::Kernel(Application::getInstance().getProgram(), "xilMM2S:{xilMM2S_2}");
    cl::Kernel *data_reader_kernel = new cl::Kernel(Application::getInstance().getProgram(), "xilS2MM:{xilS2MM_2}");

    data_writer_kernel->setArg(0, *(buffer_dec_input));
    data_writer_kernel->setArg(1, inBufferSize);
    data_writer_kernel->setArg(2, isLast);

    data_reader_kernel->setArg(0, *(buffer_dec_output));
    data_reader_kernel->setArg(1, *(buffer_size));
    data_reader_kernel->setArg(2, *(buffer_status));
    data_reader_kernel->setArg(3, outBufferSize);

    cl::CommandQueue *m_q_rd = new cl::CommandQueue(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_PROFILING_ENABLE);
    cl::CommandQueue *m_q_wr = new cl::CommandQueue(Application::getInstance().getContext(), Application::getInstance().getDevice(), CL_QUEUE_PROFILING_ENABLE);

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

    // free the cl buffers
    delete buffer_dec_input;
    delete buffer_dec_output;
    delete buffer_size;
    delete buffer_status;

    // printme("Done with decompress \n");
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