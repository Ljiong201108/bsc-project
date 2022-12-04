#include "Des.hpp"

namespace des{
std::string methodToString(Method m){
    switch(m){
        case Method::Cbc: return "Cbc";
        case Method::Cfb1: return "Cfb1";
        case Method::Cfb8: return "Cfb8";
        case Method::Cfb128: return "Cfb128";
        case Method::Ofb: return "Ofb";
        case Method::Ecb: return "Ecb";
    }
    return "";
}
}