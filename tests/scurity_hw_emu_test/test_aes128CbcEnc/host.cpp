#include "test_des.hpp"
#include "test_aes.hpp"

int main(int argc, char **argv){
    testAes::testAes128CcmEnc();
    return 0;
}