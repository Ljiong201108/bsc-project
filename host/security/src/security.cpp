#include "security.hpp"

std::string typeToString(Type t){
    switch(t){
        case Type::ENC: return "Enc";
        case Type::DEC: return "Dec";
    }
    return "";
}