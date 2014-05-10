#include "util/Checksum.hpp"

namespace trillek {
namespace util {
namespace algorithm {

static uint32_t crc32_table[256];
static bool crc32_table_computed = false;

static void GenCRC32Table()
{
    uint32_t c;
    uint32_t i;
    int k;
    for(i = 0; i < 256; i++) {
        c = i;
        for(k = 0; k < 8; k++) {
            if(c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc32_table[i] = c;
    }
    crc32_table_computed = true;
}

void Crc32::Update(const std::string &d) {
    uint32_t c = ldata;
    std::string::size_type n, l = d.length();
    if(!crc32_table_computed)
        GenCRC32Table();
    for(n = 0; n < l; n++) {
        c = crc32_table[(c ^ d[n]) & 0xff] ^ (c >> 8);
    }
    ldata = c;
}
void Crc32::Update(const void *dv, size_t l) {
    uint32_t c = ldata;
    std::string::size_type n;
    char * d = (char*)dv;
    if(!crc32_table_computed)
        GenCRC32Table();
    for(n = 0; n < l; n++) {
        c = crc32_table[(c ^ d[n]) & 0xff] ^ (c >> 8);
    }
    ldata = c;
}
void Adler32::Update(const std::string &d) {
    uint32_t c1 = ldata & 0xffff;
    uint32_t c2 = (ldata >> 16) & 0xffff;
    std::string::size_type n, l = d.length();
    for(n = 0; n < l; n++) {
        c1 = (c1 + d[n]);
        while(c1 >= 65521ul)
            c1 -= 65521;
        c2 = (c2 + c1);
        while(c2 >= 65521ul)
            c2 -= 65521;
    }
    ldata = (c2 << 16) + c1;
}
void Adler32::Update(const void *dv, size_t l) {
    uint32_t c1 = ldata & 0xffff;
    uint32_t c2 = (ldata >> 16) & 0xffff;
    char * d = (char*)dv;
    std::string::size_type n;
    for(n = 0; n < l; n++) {
        c1 = (c1 + d[n]);
        while(c1 >= 65521) c1 -= 65521;
        c2 = (c2 + c1);
        while(c2 >= 65521) c1 -= 65521;
    }
    ldata = (c2 << 16) + c1;
}

} // algorithm
} // util
} // trillek
