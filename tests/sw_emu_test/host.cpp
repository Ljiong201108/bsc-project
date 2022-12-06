// #include "testDes.hpp"
// #include "testAes.hpp"
// #include "testGzip.hpp"
// #include "testSnappy.hpp"
#include "testLz4.hpp"

int main(int argc, char **argv){
    testLz4::testLz4(argc, argv);
    return 0;
}