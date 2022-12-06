// #include "testDes.hpp"
// #include "testAes.hpp"
// #include "testGzip.hpp"
#include "testSnappy.hpp"

int main(int argc, char **argv){
    testSnappy::testSnappy(argc, argv);
    return 0;
}