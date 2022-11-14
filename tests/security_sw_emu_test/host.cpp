#include "test_des.hpp"
#include "test_aes.hpp"

int main(int argc, char **argv){
    test_aes::test_aes128CbcEnc();
    test_aes::test_aes128CbcDec();
    return 0;
}