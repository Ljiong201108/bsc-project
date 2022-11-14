#include "Aes.hpp"

namespace aes{
std::string methodToString(Method m){
    switch(m){
        case Method::Cbc: return "Cbc";
        case Method::Cfb1: return "Cfb1";
        case Method::Cfb8: return "Cfb8";
        case Method::Cfb128: return "Cfb128";
        case Method::Ofb: return "Ofb";
        case Method::Ecb: return "Ecb";
        case Method::Ccm: return "Ccm";
        case Method::Gcm: return "Gcm";
    }
    return "";
}

int keyLengthToNumBits(KeyLength k){
    switch(k){
        case KeyLength::U128: return 128;
        case KeyLength::U192: return 192;
        case KeyLength::U256: return 256;
    }
    return 0;
}

std::string keyLengthToString(KeyLength k){
    switch(k){
        case KeyLength::U128: return "128";
        case KeyLength::U192: return "192";
        case KeyLength::U256: return "256";
    }
    return 0;
}
}