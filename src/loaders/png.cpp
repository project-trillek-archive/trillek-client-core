#include "resources/pixel-buffer.hpp"
#include "util/utiltype.hpp"
#include "util/compression.hpp"
#include "util/checksum.hpp"

#include <iostream>
#include <cstdio>

namespace trillek {
namespace resource {
namespace png {

typedef uint32_t PNGLong;
typedef uint16_t PNGShort;

util::InputStream& operator>>(util::InputStream & f, PNGLong & o) {
    int c;
    for(int i = 0; i < 4 && !f.End(); i++) {
        o <<= 8;
        c = f.Read();
        if(c > -1) {
            o |= (unsigned char)c;
        }
    }
    return f;
}

util::InputStream& operator>>(util::InputStream & f, PNGShort & o) {
    int c;
    for(int i = 0; i < 2 && !f.End(); i++) {
        o <<= 8;
        c = f.Read();
        if(c > -1) {
            o |= (unsigned char)c;
        }
    }
    return f;
}

struct PNGHeader {
    PNGLong width;
    PNGLong height;
    uint8_t depth;
    uint8_t colortype;
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;
};

struct PNGTime {
    PNGShort year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

struct PNGColor {
    union {
        PNGShort value;
        struct {
            PNGShort red;
            PNGShort green;
            PNGShort blue;
        };
        uint8_t index;
    };

    uint8_t type;
    PNGColor() {
        type = 0;
        red = 0;
        green = 0;
        blue = 0;
    }
};

util::InputStream& operator>>(util::InputStream & f, PNGColor & o) {
    switch(o.type) {
    case 0:
    case 4:
        f >> o.value;
        break;
    case 2:
    case 6:
        f >> o.red >> o.green >> o.blue;
        break;
    case 3:
        f >> o.index;
        break;
    default:
        break;
    }
    return f;
}

class CrcFilter : public util::InputFilter {
public:
    CrcFilter(InputStream & f) :
        util::InputFilter(f), len(0) {
    }

    PNGLong len;
    util::FourCC type;
    util::algorithm::Crc32 crc;

    uint8_t Filter(uint8_t i) {
        crc.Update((char)i);
        len--;
        return i;
    }

    bool IsCritical() {
        return (type.cdata[0] & 32) == 0;
    }
    bool IsPrivate() {
        return (type.cdata[1] & 32) != 0;
    }
    bool IsReserved() {
        return (type.cdata[2] & 32) != 0;
    }
    bool IsCopySafe() {
        return (type.cdata[3] & 32) != 0;
    }
    void Header() {
        forward >> len >> type;
        crc.Init();
        crc.Update(type.cdata, 4);
    }
};

// base class for PNG filter types, also acts as the null filter type
class FilterType {
public:
    virtual ~FilterType() {}

    /** \brief filter a scanline with the reconstruct function
     * \param inbuffer pointer to serialized byte stream
     * \param inbuffersize max number of bytes that can be read from inbuffer
     * \param outupbuffer pointer to the previous scanline in the output buffer, is at least pixelwidth bytes long
     * \param outbuffer pointer to the buffer to write bytes to
     * \param outbuffersize max number of bytes that can be written, relative to start of outbuffer
     * \param pixelwidth number of bytes in a pixel
     * \param outstep number of bytes to advance 1 pixel, relative to the start of pixel
     * \param outupstep number of bytes to advance the outupbuffer 1 pixel
     */
    virtual void ProcessScanlineR(const uint8_t * inbuffer, size_t inbuffersize
        , const uint8_t * outupbuffer, uint8_t * outbuffer, size_t outbuffersize
        , uint32_t pixelwidth, uint32_t outstep, uint32_t outupstep) {

        size_t outpixel = 0;
        size_t pixelbyte = 0;
        size_t inbyte = 0;
        for(outpixel = 0; inbyte < inbuffersize; outpixel += outstep) {
            for(pixelbyte = 0; pixelbyte < pixelwidth; pixelbyte++) {
                if(outpixel + pixelbyte >= outbuffersize) return;
                outbuffer[outpixel + pixelbyte] = inbuffer[inbyte++];
            }
        }
    }
};

class FilterTypeSub : public FilterType {
public:
    // reconstruct with the "Sub" filter
    virtual void ProcessScanlineR(const uint8_t * inbuffer, size_t inbuffersize
        , const uint8_t * outupbuffer, uint8_t * outbuffer, size_t outbuffersize
        , uint32_t pixelwidth, uint32_t outstep, uint32_t outupstep) {

        size_t outpixel = 0;
        size_t pixelbyte = 0;
        size_t lastpixel = 0;
        size_t inbyte = 0;

        // do the first pixel special (since the previous pixel is assumed 0)
        for(pixelbyte = 0; pixelbyte < pixelwidth; pixelbyte++) {
            if(pixelbyte >= outbuffersize) return;
            outbuffer[pixelbyte] = inbuffer[inbyte++];
        }
        for(outpixel = outstep; inbyte < inbuffersize; outpixel += outstep) {
            for(pixelbyte = 0; pixelbyte < pixelwidth; pixelbyte++) {
                if(outpixel + pixelbyte >= outbuffersize) return;
                // "a" and "x" come from the PNG spec
                uint8_t x = inbuffer[inbyte++];
                uint8_t a = outbuffer[lastpixel + pixelbyte];
                outbuffer[outpixel + pixelbyte] = x + a;
            }
            lastpixel = outpixel;
        }
    }
};

class FilterTypeUp : public FilterType {
public:
    // reconstruct with the "Up" filter
    virtual void ProcessScanlineR(const uint8_t * inbuffer, size_t inbuffersize
        , const uint8_t * outupbuffer, uint8_t * outbuffer, size_t outbuffersize
        , uint32_t pixelwidth, uint32_t outstep, uint32_t outupstep) {

        size_t outpixel = 0;
        size_t pixelbyte = 0;
        size_t uppixel = 0;
        size_t inbyte = 0;

        for(outpixel = outstep; inbyte < inbuffersize; outpixel += outstep) {
            for(pixelbyte = 0; pixelbyte < pixelwidth; pixelbyte++) {
                if(outpixel + pixelbyte >= outbuffersize) return;
                // "b" and "x" come from the PNG spec
                uint8_t x = inbuffer[inbyte++];
                uint8_t b = outupbuffer[uppixel + pixelbyte];
                outbuffer[outpixel + pixelbyte] = x + b;
            }
            uppixel += outupstep;
        }
    }
};

class FilterTypeAverage : public FilterType {
public:
    // reconstruct with the "Average" filter
    virtual void ProcessScanlineR(const uint8_t * inbuffer, size_t inbuffersize
        , const uint8_t * outupbuffer, uint8_t * outbuffer, size_t outbuffersize
        , uint32_t pixelwidth, uint32_t outstep, uint32_t outupstep) {

        size_t outpixel = 0;
        size_t pixelbyte = 0;
        size_t lastpixel = 0;
        size_t uppixel = 0;
        size_t inbyte = 0;

        // do the first pixel special (since the previous pixel is assumed 0)
        for(pixelbyte = 0; pixelbyte < pixelwidth; pixelbyte++) {
            if(pixelbyte >= outbuffersize) return;
            outbuffer[pixelbyte] = inbuffer[inbyte++];
        }
        uppixel = outupstep;
        for(outpixel = outstep; inbyte < inbuffersize; outpixel += outstep) {
            for(pixelbyte = 0; pixelbyte < pixelwidth; pixelbyte++) {
                if(outpixel + pixelbyte >= outbuffersize) return;
                // "b", "a", "x" come from the PNG spec
                uint8_t x = inbuffer[inbyte++];
                uint32_t a = outbuffer[lastpixel + pixelbyte];
                uint32_t b = outupbuffer[uppixel + pixelbyte];
                outbuffer[outpixel + pixelbyte] = x + (uint8_t)((a + b) >> 1);
            }
            lastpixel = outpixel;
            uppixel += outupstep;
        }
    }
};

static __inline uint8_t PaethFunction(uint8_t a, uint8_t b, uint8_t c) {
    int32_t p = a;
    p += b;
    p -= c;
    int32_t pa, pb, pc;
    pa = p - (int32_t)a;
    pb = p - (int32_t)b;
    pc = p - (int32_t)c;
    pa = pa < 0 ? -pa : pa;
    pb = pb < 0 ? -pb : pb;
    pc = pc < 0 ? -pc : pc;
    if(pa <= pb && pa <= pc) return a;
    if(pb <= pc) return b;
    return c;
}

class FilterTypePaeth : public FilterType {
public:
    // reconstruct with the "Paeth" filter
    virtual void ProcessScanlineR(const uint8_t * inbuffer, size_t inbuffersize
        , const uint8_t * outupbuffer, uint8_t * outbuffer, size_t outbuffersize
        , uint32_t pixelwidth, uint32_t outstep, uint32_t outupstep) {

        size_t outpixel = 0;
        size_t pixelbyte = 0;
        size_t lastpixel = 0;
        size_t uppixel = 0;
        size_t lastuppixel = 0;
        size_t inbyte = 0;

        // do the first pixel special (since the previous pixel is assumed 0)
        for(pixelbyte = 0; pixelbyte < pixelwidth; pixelbyte++) {
            if(pixelbyte >= outbuffersize) return;
            outbuffer[pixelbyte] = inbuffer[inbyte++];
        }
        uppixel = outupstep;
        for(outpixel = outstep; inbyte < inbuffersize; outpixel += outstep) {
            for(pixelbyte = 0; pixelbyte < pixelwidth; pixelbyte++) {
                if(outpixel + pixelbyte >= outbuffersize) return;
                // "c", "b", "a", "x" come from the PNG spec
                uint8_t x = inbuffer[inbyte++];
                uint32_t a = outbuffer[lastpixel + pixelbyte];
                uint32_t b = outupbuffer[uppixel + pixelbyte];
                uint32_t c = outupbuffer[lastuppixel + pixelbyte];
                outbuffer[outpixel + pixelbyte] = x + PaethFunction(a, b, c);
            }
            lastpixel = outpixel;
            lastuppixel = uppixel;
            uppixel += outupstep;
        }
    }
};

util::void_er Load(util::InputStream & f, resource::PixelBuffer & pix) {
    using util::void_er;
    using util::FourCC;
    const unsigned char magic[] = {137, 80, 78, 71, 13, 10, 26, 10};

    std::map<uint32_t, uint32_t> chunkcounts;

    PNGLong inlong;
    PNGLong gama;
    PNGColor trans;
    PNGColor background;
    PNGTime mtime;
    PNGHeader header;
    CrcFilter chunk(f);
    util::algorithm::Inflate decoder;

    void_er stat;
    int i;
    bool valid = true;
    bool needheader = true;
    bool needpalette = false;
    bool idatmode = false;
    bool at_end = false;

    // check magic number
    for(i = 0; i < 8 && !f.End(); i++) {
        if(magic[i] != f.Read()) {
            valid = false;
        }
    }
    if(f.End()) return void_er(-1, "Unexpected end of file");
    if(!valid || i < 8) return void_er(-1, "Bad magic number");

    // process chunks
    while(!f.End() && !stat && !at_end) {
        chunk.Header();
        std::cerr << chunk.type << ": " << chunk.len << '\n';
        if(chunk.type == FourCC("IDAT")) {
            if(!idatmode) {
                decoder.DecompressStart();
                idatmode = true;
            }
            uint8_t c;
            util::DataString compresseddata;
            while(!f.End() && chunk.len > 0) {
                chunk >> c;
                compresseddata.append(&c, 1);
            }
            decoder.DecompressData(compresseddata);
        }
        else if(chunk.type == FourCC("IEND")) {
            at_end = true;
        }
        else if(idatmode) {
            return void_er(-1, "Invalid chunk in image stream");
        }
        else if(chunk.type == FourCC("IHDR")) {
            if(needheader) {
                chunkcounts[chunk.type.ldata] = 1;
                chunk >> header.width >> header.height >> header.depth;
                chunk >> header.colortype >> header.compression;
                chunk >> header.filter >> header.interlace;
                if((header.width | header.height) > (1 << 23)) {
                    return void_er(-1, "Image size out of bounds");
                }
                switch(header.colortype) {
                case 0:
                    switch(header.depth) {
                    case 1:
                    case 2:
                    case 4:
                    case 8:
                    case 16:
                        // valid
                        break;
                    default:
                        return void_er(-1, "Invalid color depth");
                    }
                    break;
                case 3:
                    needpalette = true;
                    switch(header.depth) {
                    case 1:
                    case 2:
                    case 4:
                    case 8:
                        // valid
                        break;
                    default:
                        return void_er(-1, "Invalid color depth");
                    }
                    break;
                case 2:
                case 4:
                case 6:
                    if(header.depth != 8 && header.depth != 16) {
                        return void_er(-1, "Invalid color depth");
                    }
                    break;
                default:
                    return void_er(-1, "Invalid color type");
                    break;
                }
                if(header.filter != 0) {
                    return void_er(-1, "Invalid/unsupported filter method");
                }
                if(header.compression != 0) {
                    return void_er(-1, "Invalid/unsupported compression method");
                }
                if(header.interlace != 0 && header.interlace != 1) {
                    return void_er(-1, "Invalid/unsupported interlace method");
                }
                // XXX: debug info
                std::fprintf(stderr, "Image is %d x %d @%d\n", header.width, header.height, header.depth);
                std::fprintf(stderr, "Color: %d\nCompressor: %d\nFilter: %d\nInterlace: %d\n",
                        header.colortype, header.compression, header.filter, header.interlace);
                needheader = false;
            } else {
                return void_er(-1, "Multiple header chunks");
            }
        }
        else if(chunk.type == FourCC("gAMA")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple gAMA chunk");
            }
            chunk >> gama;
            std::cerr << "Gama: " << gama << '\n';
            chunkcounts[chunk.type.ldata] = 1;
        }
        else if(chunk.type == FourCC("tIME")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple tIME chunk");
            }
            chunkcounts[chunk.type.ldata] = 1;

            chunk >> mtime.year >> mtime.month >> mtime.day;
            chunk >> mtime.hour >> mtime.minute >> mtime.second;
            // XXX: debug info
            std::fprintf(stderr, "Modification: %d:%d:%d %dD %dM %dY\n",mtime.hour, mtime.minute, mtime.second, mtime.day, mtime.month, mtime.year);
        }
        else if(chunk.type == FourCC("bKGD")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple bKGD chunk");
            }
            chunkcounts[chunk.type.ldata] = 1;

            background.type = header.colortype;
            chunk >> background;
            // XXX: debug info
            if(background.type == 0 || background.type == 4) std::fprintf(stderr, "Background: Y %d\n", background.value);
            if(background.type == 2 || background.type == 6) {
                std::fprintf(stderr, "Background: RGB %d %d %d\n", background.red, background.green, background.blue);
            }
            if(background.type == 3) std::fprintf(stderr, "Background: I %d\n", background.index);
        }
        else if(chunk.type == FourCC("pHYs")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple pHYs chunk");
            }
            chunkcounts[chunk.type.ldata] = 1;
            PNGLong pix_x, pix_y;
            uint8_t unit;
            chunk >> pix_x >> pix_y >> unit;
            // XXX: debug info
            std::fprintf(stderr, "Pixelsize: %d x %d ", pix_x, pix_y);
            if(unit == 1) {
                std::fprintf(stderr, "pixels/m\n");
            }
            else {
                std::fprintf(stderr, "unknown\n");
            }
        }
        else if(chunk.type == FourCC("tRNS")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple tRNS chunk");
            }
            chunkcounts[chunk.type.ldata] = 1;

            trans.type = header.colortype;
            chunk >> trans;
        }
        else if(chunk.type == FourCC("tEXt")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                chunkcounts[chunk.type.ldata] += 1;
            } else {
                chunkcounts[chunk.type.ldata] = 1;
            }
            std::string keyword;
            std::string textdata;
            uint8_t c;

            while(!f.End() && chunk.len > 0) {
                chunk >> c;
                if(c == 0) {
                    break;
                }
                else {
                    keyword.append((char*)&c, 1);
                }
            }
            while(!f.End() && chunk.len > 0) {
                chunk >> c;
                textdata.append((char*)&c, 1);
            }
            // We do nothing with the text
        }
        else if(chunk.type == FourCC("zTXt")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                chunkcounts[chunk.type.ldata] += 1;
            } else {
                chunkcounts[chunk.type.ldata] = 1;
            }
            std::string keyword;
            util::DataString compresseddata;
            util::DataString textdata;
            uint8_t method = 0;
            uint8_t c;

            while(!f.End() && chunk.len > 0) {
                chunk >> c;
                if(c == 0) {
                    break;
                }
                else {
                    keyword.append((char*)&c, 1);
                }
            }
            if(!f.End() && chunk.len > 0) {
                chunk >> method;
            }
            while(!f.End() && chunk.len > 0) {
                chunk >> c;
                compresseddata.append(&c, 1);
            }
            if(method != 0) {
                stat = void_er(0, "Bad compression on text");
            }
            util::algorithm::Inflate inf;
            inf.DecompressStart();
            inf.DecompressData(compresseddata);
            inf.DecompressEnd();
            textdata = inf.DecompressGetOutput();
            // We do nothing with the text
        }
        else {
            if(chunk.IsCritical()) {
                return void_er(-1, "Unsupported critical chunk");
            }
        }
        while(!f.End() && chunk.len > 0) {
            chunk.crc.Update(f.Read());
            chunk.len--;
        }
        f >> inlong; // get the CRC
        chunk.crc.Last();
        if(chunk.crc.ldata != inlong) {
            std::cerr << inlong << "!=" << chunk.crc.ldata << '\n';
            return void_er(-1, "CRC Failure");
        }
    }
    return stat;
}

} // png
} // resource
} // trillek
