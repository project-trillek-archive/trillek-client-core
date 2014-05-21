#ifndef CHECKSUM_HPP_INCLUDED
#define CHECKSUM_HPP_INCLUDED

#include "utiltype.hpp"

namespace trillek {
namespace util {
namespace algorithm {

struct Crc32 : public FourCC {
    Crc32() {
        Init();
    }
    void Init() {
        ldata = 0xffffffffl;
    }
    void Update(char d);
    void Update(const std::string &d);
    void Update(const void *d, size_t l);
    void Last() {
        ldata ^= 0xffffffffl;
    }
    void Full(const std::string &d) {
        Init();
        Update(d);
        Last();
    }
};
struct Adler32 : public FourCC {
    Adler32() {
        Init();
    }
    void Init() {
        ldata = 0x1;
    }
    void Update(const std::string &d);
    void Update(const void *d, size_t l);
    void Last() { }
    void Full(const std::string &d) {
        Init();
        Update(d);
    }
};

}
}
}
#endif
