#include "util/checksum.hpp"

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
                c = 0xedb88320ul ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc32_table[i] = c;
    }
    crc32_table_computed = true;
}
void Crc32::Update(char d) {

    if(!crc32_table_computed)
        GenCRC32Table();
    ldata = crc32_table[(ldata ^ ((unsigned char)d)) & 0xff] ^ (ldata >> 8);

}
void Crc32::Update(const std::string &d) {
    uint32_t c = ldata;
    std::string::size_type n, l = d.length();
    if(!crc32_table_computed)
        GenCRC32Table();
    for(n = 0; n < l; n++) {
        c = crc32_table[(c ^ ((unsigned char)d[n])) & 0xff] ^ (c >> 8);
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
        c = crc32_table[(c ^ ((unsigned char)d[n])) & 0xff] ^ (c >> 8);
    }
    ldata = c;
}

static const uint32_t ADLER_LIMIT = 5552;
static const uint32_t ADLER_BASE = 65521u;
void Adler32::Update(const std::string &d) {
    Update(d.data(), d.length());
}
void Adler32::Update(const void *dv, size_t l) {
    uint32_t c1 = ldata & 0xffff;
    uint32_t c2 = (ldata >> 16) & 0xffff;
    unsigned char * d = (unsigned char*)dv;
    std::string::size_type n = 0;
    while(l >= ADLER_LIMIT) {
        l -= ADLER_LIMIT;
        uint32_t limit = ADLER_LIMIT / 16;
        while(limit--) {
            c1 += d[n   ]; c2 += c1; c1 += d[n+ 1]; c2 += c1;
            c1 += d[n+ 2]; c2 += c1; c1 += d[n+ 3]; c2 += c1;
            c1 += d[n+ 4]; c2 += c1; c1 += d[n+ 5]; c2 += c1;
            c1 += d[n+ 6]; c2 += c1; c1 += d[n+ 7]; c2 += c1;
            c1 += d[n+ 8]; c2 += c1; c1 += d[n+ 9]; c2 += c1;
            c1 += d[n+10]; c2 += c1; c1 += d[n+11]; c2 += c1;
            c1 += d[n+12]; c2 += c1; c1 += d[n+13]; c2 += c1;
            c1 += d[n+14]; c2 += c1; c1 += d[n+15]; c2 += c1;
            n += 16;
        }
        c1 %= ADLER_BASE;
        c2 %= ADLER_BASE;
    }
    for(; l; n++, l--) {
        c1 += d[n];
        while(c1 >= ADLER_BASE) {
            c1 -= ADLER_BASE;
        }
        c2 += c1;
        while(c2 >= ADLER_BASE) {
            c2 -= ADLER_BASE;
        }
    }
    ldata = (c2 << 16) + c1;
}

} // algorithm
} // util
} // trillek
