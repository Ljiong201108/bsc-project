#include "test_des.hpp"
#include "test_aes.hpp"

int main(int argc, char **argv){
    testAes::testAes128CbcEnc();
    testAes::testAes128CbcDec();
    return 0;
}