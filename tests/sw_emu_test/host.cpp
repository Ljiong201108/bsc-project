// #include "testDes.hpp"
// #include "testAes.hpp"
#include "testGzip.hpp"

// #include "gzipApp.hpp"
// #include "gzipOCLHost.hpp"
// #include <memory>

int main(int argc, char **argv){
    testGzip::test();

	// bool enable_profile = true;
    // compressBase::State flow = compressBase::BOTH;
    // gzipBase::d_type decKernelType = gzipBase::FULL;
    // bool swpipline_option = true;

    // // Driver class object
    // gzipApp d(argc, argv, swpipline_option);

    // // Design class object creating and constructor invocation
    // std::unique_ptr<gzipOCLHost> gzip(
    //     new gzipOCLHost(flow, d.getXclbin(), d.getDeviceId(), decKernelType, d.getDesignFlow()));

    // // Run API to launch the compress or decompress engine
    // d.run(gzip.get(), enable_profile);
    return 0;
}