#include "resources/PixelBuffer.hpp"
#include "util/Compression.hpp"
#include "util/Checksum.hpp"

namespace trillek {
namespace resource {

struct PNGChunk {
    uint32_t len;
    util::FourCC type;
    util::algorithm::CRC32 crc;

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

int LoadPNG(std::istream &f, PixelBuffer &pix) {
    const unsigned char magic[] = {137, 80, 78, 71, 13, 10, 26, 10};

    int i;
    bool valid = true;

    for(i = 0; i < 8 && !f.eof(); i++) {
        if(magic[i] != f.get()) {
            valid = false;
        }
    }
    if(!valid || i < 8) return -1;

    return 0;
}

} // resource
} // trillek
