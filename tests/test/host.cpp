// #include "testDes.hpp"
// #include "testAes.hpp"
#include "testGzip.hpp"
#include "testSnappy.hpp"
#include "testLz4.hpp"
#include "testZstd.hpp"

int main(int argc, char **argv){
    // testGzip::testGzipCompress();
    testGzip::testGzipDecompress();
    // testGzip::testGzipSimple();
    // testGzip::testZlib();
    // testSnappy::testSnappy(argc, argv);
    // testLz4::testLz4(argc, argv);
    // testZstd::testZstd(argc, argv);
    return 0;
}