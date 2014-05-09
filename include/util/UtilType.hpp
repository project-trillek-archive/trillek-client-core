
#ifndef UTILTYPE_HPP_INCLUDED
#define UTILTYPE_HPP_INCLUDED

#include <string>
#include <istream>
#include <stdint.h>

namespace trillek {
namespace util {

struct FourCC {
    union {
        char cdata[4];
        uint32_t ldata;
    };
    FourCC();
    FourCC(char a, char b, char c, char d);
    FourCC(const char * a);
    FourCC(std::istream &f);
    bool operator==(const FourCC r) {
        return ldata == r.ldata;
    }
    bool operator!=(const FourCC r) {
        return ldata != r.ldata;
    }
};

uint16_t BitReverse16(uint16_t n);
uint32_t BitReverse32(uint32_t n);
uint32_t BitReverse(uint32_t v, int bits);

std::istream &operator>>(std::istream &f, FourCC &o);

} // util
} // trillek
#endif
