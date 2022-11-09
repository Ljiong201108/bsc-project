#include "security.hpp"

std::string typeToString(Type t){
    switch(t){
        case Type::ENC: return "ENC";
        case Type::DEC: return "DEC";
    }
    return "";
}