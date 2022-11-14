#include "test_des.hpp"
#include "test_aes.hpp"

int main(int argc, char **argv){
    testAes::testAes128CbcEnc();
    testAes::testAes128CbcDec();
    testAes::testAes128CcmEnc();
    testAes::testAes128CcmDec();
    return 0;
}