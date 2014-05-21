#include "resources/pixel-buffer.hpp"
#include "util/utiltype.hpp"
#include "util/compression.hpp"
#include "util/checksum.hpp"

#include <iostream>

namespace trillek {
namespace resource {

struct PNGLong {
    uint32_t value;
};
struct PNGChunk {
    PNGLong len;
    util::FourCC type;
    //util::algorithm::Crc32 crc;

    bool IsCritical() {
        return (type.cdata[0] & 32) == 0;
    }
    bool IsPrivate() {
        return (type.cdata[1] & 32) != 0;
    }
    bool IsReserved() {
        return (type.cdata[2] & 32) != 0;
    }
};
struct PNGHeader {
    PNGLong width;
    PNGLong height;
    uint8_t depth;
    uint8_t colortype;
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;
};

class CrcFilter : public util::InputFilter {
public:
    CrcFilter(InputStream & f) :
        util::InputFilter(f) {
    }

    uint8_t filter(uint8_t i) {
        crc.Update((char)i);
        return i;
    }

    util::algorithm::Crc32 crc;
};

util::InputStream& operator>>(util::InputStream &f, PNGLong &o) {
    int c;
    for(int i = 0; i < 4 && !f.eof(); i++) {
        o.value <<= 8;
        c = f.read();
        if(c > -1) {
            o.value |= (unsigned char)c;
        }
    }
    return f;
}

util::InputStream& operator>>(util::InputStream &f, PNGChunk &o) {
    f >> o.len >> o.type;
    return f;
}

util::void_er LoadPNG(std::istream &datastream, PixelBuffer &pix) {
    using util::void_er;
    using util::FourCC;
    const unsigned char magic[] = {137, 80, 78, 71, 13, 10, 26, 10};

    std::map<uint32_t, uint32_t> chunkcounts;
    PNGChunk chunk;
    PNGLong inlong;
    PNGHeader header;
    util::StdInputStream f(datastream);
    CrcFilter crcf(f);

    void_er stat;
    int i;
    bool valid = true;
    bool needheader = true;
    bool idatmode = false;

    // check magic number
    for(i = 0; i < 8 && !f.eof(); i++) {
        if(magic[i] != f.read()) {
            valid = false;
        }
    }
    if(f.eof()) return void_er(-1, "Unexpected end of file");
    if(!valid || i < 8) return void_er(-1, "Bad magic number");

    // process chunks
    while(!f.eof() && !stat) {
        f >> chunk;
        crcf.crc.Init();
        crcf.crc.Update(chunk.type.cdata, 4);
        std::cerr << chunk.type << ": " << chunk.len.value << '\n';
        if(chunk.type == FourCC("IDAT")) {
        }
        else if(chunk.type == FourCC("IHDR")) {
            if(needheader) {
                chunkcounts[chunk.type.ldata] = 1;
                crcf >> header.width >> header.height >> header.depth;
                crcf >> header.colortype >> header.compression;
                crcf >> header.filter >> header.interlace;
                chunk.len.value -= 13;
                needheader = false;
            } else {
                return void_er(-1, "Multiple header chunks");
            }
        }
        while(!f.eof() && chunk.len.value > 0) {
            crcf.crc.Update(f.read());
            chunk.len.value--;
        }
        f >> inlong;
        crcf.crc.Last();
        if(crcf.crc.ldata != inlong.value) {
            std::cerr << inlong.value << "!=" << crcf.crc.ldata << '\n';
            return void_er(-1, "CRC Failure");
        }
    }
    return stat;
}

} // resource
} // trillek
