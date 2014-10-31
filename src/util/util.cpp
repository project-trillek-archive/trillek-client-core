#include "util/utiltype.hpp"

namespace trillek {
namespace util {

FourCC::FourCC() {
    ldata = 0;
}
FourCC::FourCC(char a, char b, char c, char d) {
    cdata[0] = a;
    cdata[1] = b;
    cdata[2] = c;
    cdata[3] = d;
}
FourCC::FourCC(const char * a) {
    cdata[0] = a[0];
    cdata[1] = a[1];
    cdata[2] = a[2];
    cdata[3] = a[3];
}
FourCC::FourCC(std::istream &f) {
    for(int i = 0; i < 4 && !f.eof(); i++) {
        cdata[i] = f.get();
    }
}

uint16_t BitReverse16(uint16_t n)
{
    n = ((n & 0xAAAA) >> 1) | ((n & 0x5555) << 1);
    n = ((n & 0xCCCC) >> 2) | ((n & 0x3333) << 2);
    n = ((n & 0xF0F0) >> 4) | ((n & 0x0F0F) << 4);
    n = ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
    return n;
}
uint32_t BitReverse32(uint32_t n)
{
    n = ((n & 0xAAAAAAAA) >> 1) | ((n & 0x55555555) << 1);
    n = ((n & 0xCCCCCCCC) >> 2) | ((n & 0x33333333) << 2);
    n = ((n & 0xF0F0F0F0) >> 4) | ((n & 0x0F0F0F0F) << 4);
    n = ((n & 0xFF00FF00) >> 8) | ((n & 0x00FF00FF) << 8);
    n = ((n & 0xFFFF0000) >>16) | ((n & 0x0000FFFF) <<16);
    return n;
}
uint32_t BitReverse(uint32_t v, int bits)
{
    if(bits > 32) return 0;
    return BitReverse32(v) >> (32 - bits);
}

std::istream &operator>>(std::istream &f, FourCC &o) {
    for(int i = 0; i < 4 && !f.eof(); i++) {
        o.cdata[i] = f.get();
    }
    return f;
}
InputStream& operator>>(InputStream & f, FourCC & o) {
    for(int i = 0; i < 4 && !f.End(); i++) {
        o.cdata[i] = f.Read();
    }
    return f;
}
InputStream& operator>>(InputStream & f, uint8_t & o) {
    o = f.Read();
    return f;
}
std::ostream &operator<<(std::ostream &f, FourCC &o) {
    f << o.cdata[0];
    f << o.cdata[1];
    f << o.cdata[2];
    f << o.cdata[3];
    return f;
}
} // util
} // trillek
