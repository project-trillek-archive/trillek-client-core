#include "resources/pixel-buffer.hpp"
#include "util/utiltype.hpp"
#include "util/compression.hpp"
#include "util/checksum.hpp"

#include <iostream>
#include <cstdio>

// Experimental / Debug options
//#define PNG_PROGRESSIVE_DISPLAY
//#define PNG_BEBUG_PASSES
//#define PNG_DEBUG_OUTPUT

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

struct PNGPaletteEntry {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
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

        for(outpixel = 0; inbyte < inbuffersize; outpixel += outstep) {
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
            uint8_t x = inbuffer[inbyte++];
            uint32_t b = outupbuffer[pixelbyte];
            outbuffer[pixelbyte] = x + (uint8_t)(b >> 1);
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

// Paeth function from the PNG spec
static __inline uint8_t PaethFunction(uint8_t a, uint8_t b, uint8_t c) {
    int32_t p = a;
    p += b;
    p -= c;
    int32_t pa, pb, pc;
    pa = p - a;
    pb = p - b;
    pc = p - c;
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
            uint8_t x = inbuffer[inbyte++];
            uint32_t b = outupbuffer[uppixel + pixelbyte];
            outbuffer[pixelbyte] = x + PaethFunction(0, b, 0);
        }
        uppixel = outupstep;
        for(outpixel = outstep; inbyte < inbuffersize; outpixel += outstep) {
            for(pixelbyte = 0; pixelbyte < pixelwidth; pixelbyte++) {
                if(outpixel + pixelbyte >= outbuffersize) return;
                // "c", "b", "a", "x" come from the PNG spec
                uint8_t x = inbuffer[inbyte++];
                uint8_t a = outbuffer[lastpixel + pixelbyte];
                uint8_t b = outupbuffer[uppixel + pixelbyte];
                uint8_t c = outupbuffer[lastuppixel + pixelbyte];
                outbuffer[outpixel + pixelbyte] = x + PaethFunction(a, b, c);
            }
            lastpixel = outpixel;
            lastuppixel = uppixel;
            uppixel += outupstep;
        }
    }
};

struct FilterMethodTable {
    uint32_t count;
    FilterType * items[10];
};

class InterlaceType {
public:
    FilterMethodTable & filters;
    const PNGHeader & header;
    uint32_t pixelbytes;
    uint32_t pixelsperbyte;
    uint32_t steppixelbytes;
    uint32_t linelength;
    uint32_t pass;
    uint32_t line;
    uint32_t inpos;
    const PNGPaletteEntry *pal;
    uint32_t palcount;

    InterlaceType(FilterMethodTable & ftable, const PNGHeader & head)
        : filters(ftable), header(head) {
        pass = 0;
        line = 0;
        inpos = 0;
        pixelsperbyte = 0;
        palcount = 0;
        pal = nullptr;
        switch(header.depth) {
        case 1:
            pixelbytes = 1;
            pixelsperbyte = 8;
            if(header.colortype == 3) {
                steppixelbytes = 4 * pixelsperbyte;
            }
            else {
                steppixelbytes = pixelsperbyte;
            }
            linelength  = (header.width / pixelsperbyte);
            linelength += (header.width & (pixelsperbyte - 1) ? 1 : 0);
            break;
        case 2:
            pixelbytes = 1;
            pixelsperbyte = 4;
            if(header.colortype == 3) {
                steppixelbytes = 4 * pixelsperbyte;
            }
            else {
                steppixelbytes = pixelsperbyte;
            }
            linelength  = (header.width / pixelsperbyte);
            linelength += (header.width & (pixelsperbyte - 1) ? 1 : 0);
            break;
        case 4:
            pixelbytes = 1;
            pixelsperbyte = 2;
            if(header.colortype == 3) {
                steppixelbytes = 4 * pixelsperbyte;
            }
            else {
                steppixelbytes = pixelsperbyte;
            }
            linelength  = (header.width / pixelsperbyte);
            linelength += (header.width & (pixelsperbyte - 1) ? 1 : 0);
            break;
        case 8:
        case 16:
            switch(header.colortype) {
            case 0:
                pixelbytes = (header.depth / 8);
                steppixelbytes = pixelbytes;
                break;
            case 2:
                pixelbytes = 3 * (header.depth / 8);
                steppixelbytes = pixelbytes;
                break;
            case 3:
                pixelbytes = 1;
                steppixelbytes = 4;
                break;
            case 4:
                pixelbytes = 2 * (header.depth / 8);
                steppixelbytes = pixelbytes;
                break;
            case 6:
                pixelbytes = 4 * (header.depth / 8);
                steppixelbytes = pixelbytes;
                break;
            default:
                pixelbytes = 0;
                steppixelbytes = pixelbytes;
                break;
            }
            linelength = header.width * pixelbytes;
            break;
        default:
            linelength = 0;
            steppixelbytes = 0;
            pixelbytes = 0;
            break;
        }
    }
    virtual ~InterlaceType() {}

    void AssignPalette(const PNGPaletteEntry * p, uint32_t psize) {
        pal = p;
        palcount = psize;
    }

    virtual uint32_t Deindex(uint8_t * pixel_ptr, resource::PixelBuffer & pix) {
        uint8_t * pixelline_ptr;
        uint8_t * pixellimit_ptr = pixel_ptr + (header.height * pix.Pitch());
        uint8_t pixeldata;
        uint8_t colorindex;
        uint32_t subpixel;
        uint32_t outlinelength = header.width * 4;
        uint32_t indexstep = 4;
        uint32_t shiftmax = pixelsperbyte - 1;
        uint32_t shiftsize;
        if(pixelsperbyte) {
            shiftsize = 8 / pixelsperbyte;
        }
        else {
            shiftsize = 0;
        }
        if(pixel_ptr == nullptr || pal == nullptr || palcount < 1) {
            return 5;
        }

        pixelline_ptr = pixel_ptr;
        for(line = 0; line < header.height; line++) {
            subpixel = 0;
            if(pixelsperbyte > 1) {
                for(uint32_t colpixel = 0; colpixel < outlinelength; ) {
                    for(subpixel = 0, pixeldata = pixelline_ptr[colpixel];
                        subpixel < pixelsperbyte && colpixel < outlinelength;
                        subpixel++, colpixel += indexstep) {
                        colorindex  = pixeldata >> (shiftsize * (shiftmax - subpixel));
                        colorindex &= (1 << shiftsize) - 1;
                        if(pixelline_ptr + colpixel + 3 >= pixellimit_ptr) {
                            return 1;
                        }
                        if(colorindex < palcount) {
                            pixelline_ptr[colpixel  ] = pal[colorindex].red;
                            pixelline_ptr[colpixel+1] = pal[colorindex].green;
                            pixelline_ptr[colpixel+2] = pal[colorindex].blue;
                            pixelline_ptr[colpixel+3] = pal[colorindex].alpha;
                        }
                    }
                }
            }
            else {
                for(uint32_t colpixel = 0; colpixel < outlinelength; colpixel += indexstep) {
                    colorindex = pixelline_ptr[colpixel];
                    if(colorindex < palcount) {
                        pixelline_ptr[colpixel  ] = pal[colorindex].red;
                        pixelline_ptr[colpixel+1] = pal[colorindex].green;
                        pixelline_ptr[colpixel+2] = pal[colorindex].blue;
                        pixelline_ptr[colpixel+3] = pal[colorindex].alpha;
                    }
                }
            }
            pixelline_ptr += pix.Pitch();
        }
        return 0;
    }

    virtual uint32_t Deinterlace(const util::DataString & linedata, resource::PixelBuffer & pix) {
        // 1 byte at beginning of every line + the lines
        if(linedata.length() < header.height + (header.height * linelength)) {
            return 0;
        }
        uint32_t ft;
        uint8_t * pixel_ptr = pix.LockWrite();
        uint8_t zero[16];
        uint8_t * pixelline_ptr;
        uint8_t * pixelupline_ptr;
        uint32_t maxlen = linedata.length();
        uint32_t outlinelength = header.width * steppixelbytes;

        if(pixel_ptr == nullptr) {
            return 0;
        }
        for(int i = 0; i < 16; i++) zero[i] = 0;
        if(0 == line) {
            ft = linedata[inpos++];
            if(inpos + linelength <= maxlen) {
                if(ft < filters.count) {
                    filters.items[ft]->ProcessScanlineR(linedata.data() + inpos
                        , linelength, zero, pixel_ptr, outlinelength
                        , pixelbytes, steppixelbytes, 0);
                }
            }
            inpos += linelength;
        }
        pixelupline_ptr = pixel_ptr;
        pixelline_ptr = pixel_ptr + pix.Pitch();
        for(;line < header.height; line++) {
            ft = linedata[inpos++];
            if(inpos + linelength > maxlen) {
                break;
            }
            if(ft < filters.count) {
                filters.items[ft]->ProcessScanlineR(linedata.data() + inpos
                    , linelength, pixelupline_ptr, pixelline_ptr, outlinelength
                    , pixelbytes, steppixelbytes, steppixelbytes);
            }
            pixelupline_ptr = pixelline_ptr;
            pixelline_ptr += pix.Pitch();
            inpos += linelength;
        }
        if(header.colortype == 3) {
            Deindex(pixel_ptr, pix);
        }
        pix.UnlockWrite();
        return 0;
    }
};

class InterlaceTypeAdam7 : public InterlaceType {
public:
    static const uint32_t pass_coloffset[7];
    static const uint32_t pass_colstep[7];
    static const uint32_t pass_rowoffset[7];
    static const uint32_t pass_rowstep[7];

    InterlaceTypeAdam7(FilterMethodTable & ftable, const PNGHeader & head)
        : InterlaceType(ftable, head) {}

    virtual uint32_t Deindex(uint8_t * pixel_ptr, resource::PixelBuffer & pix) {
        uint8_t * pixelline_ptr;
        uint8_t * pixellimit_ptr = pixel_ptr + (header.height * pix.Pitch());
        uint8_t pixeldata;
        uint8_t colorindex;
        uint32_t subpixel;
        uint32_t outlinelength = header.width * 4;
        uint32_t indexstep = (pass_colstep[pass] * 4);
        uint32_t shiftmax = pixelsperbyte - 1;
        uint32_t shiftsize;
        if(pixelsperbyte) {
            shiftsize = 8 / pixelsperbyte;
        }
        else {
            shiftsize = 0;
        }
        if(pixel_ptr == nullptr || pal == nullptr || palcount < 1) {
            return 5;
        }

        pixelline_ptr = pixel_ptr;
        pixelline_ptr += pass_rowoffset[pass] * pix.Pitch();
        for(line = pass_rowoffset[pass]; line < header.height; line += pass_rowstep[pass]) {
            subpixel = 0;
            if(pixelsperbyte > 1) {
                for(uint32_t colpixel = pass_coloffset[pass] * 4; colpixel < outlinelength; ) {
                    for(subpixel = 0, pixeldata = pixelline_ptr[colpixel];
                        subpixel < pixelsperbyte && colpixel < outlinelength;
                        subpixel++, colpixel += indexstep) {
                        colorindex  = pixeldata >> (shiftsize * (shiftmax - subpixel));
                        colorindex &= (1 << shiftsize) - 1;
                        if(pixelline_ptr + colpixel + 3 >= pixellimit_ptr) {
                            return 1;
                        }
                        if(colorindex < palcount) {
                            pixelline_ptr[colpixel  ] = pal[colorindex].red;
                            pixelline_ptr[colpixel+1] = pal[colorindex].green;
                            pixelline_ptr[colpixel+2] = pal[colorindex].blue;
                            pixelline_ptr[colpixel+3] = pal[colorindex].alpha;
                        }
                    }
                }
            }
            else {
                for(uint32_t colpixel = pass_coloffset[pass] * 4;
                        colpixel < outlinelength; colpixel += indexstep) {
                    colorindex = pixelline_ptr[colpixel];
                    if(colorindex < palcount) {
                        pixelline_ptr[colpixel  ] = pal[colorindex].red;
                        pixelline_ptr[colpixel+1] = pal[colorindex].green;
                        pixelline_ptr[colpixel+2] = pal[colorindex].blue;
                        pixelline_ptr[colpixel+3] = pal[colorindex].alpha;
                    }
                }
            }
            pixelline_ptr += pass_rowstep[pass] * pix.Pitch();
        }
        return 0;
    }

    virtual uint32_t Deinterlace(const util::DataString & linedata, resource::PixelBuffer & pix) {
        uint32_t ft;

        uint32_t passlinelength;
        uint8_t * pixel_ptr = pix.LockWrite();
        uint8_t zero[16];
        uint8_t * pixelline_ptr;
        uint8_t * pixelupline_ptr;
        uint32_t maxlen = linedata.length();
        uint32_t offsetcolbytes;
        uint32_t stepcolbytes;

        if(pixel_ptr == nullptr) {
            return 0;
        }
        for(int i = 0; i < 16; i++) zero[i] = 0;

        for(;pass < 7; pass++) {
            passlinelength = (header.width-pass_coloffset[pass]) / pass_colstep[pass];
            if((header.width - pass_coloffset[pass]) & (pass_colstep[pass] - 1)) {
                passlinelength++;
            }
            //if(pass_coloffset[pass] > ((header.width) & (pass_colstep[pass] - 1))) {
            //	passlinelength--;
            //}
            if(pixelsperbyte) {
                if(passlinelength & (pixelsperbyte - 1)) {
                    passlinelength += pixelsperbyte;
                }
                passlinelength /= pixelsperbyte;
            }
            passlinelength *= pixelbytes;
            if(pixelsperbyte > 1) {
                offsetcolbytes = pass_coloffset[pass] * 4;
            }
            else {
                offsetcolbytes = pass_coloffset[pass] * steppixelbytes;
            }
            stepcolbytes = pass_colstep[pass] * steppixelbytes;
            pixelline_ptr = pixel_ptr + offsetcolbytes + (pix.Pitch() * pass_rowoffset[pass]);

            ft = linedata[inpos++];
            if(inpos + passlinelength > maxlen) {
                pix.UnlockWrite();
                return 0;
            }
            if(ft < filters.count) {
                filters.items[ft]->ProcessScanlineR(linedata.data() + inpos, passlinelength
                    , zero, pixelline_ptr
                    , (header.width * steppixelbytes) - offsetcolbytes
                    , pixelbytes, stepcolbytes, 0);
            }
            inpos += passlinelength;
            pixelupline_ptr = pixelline_ptr;
            pixelline_ptr += (pix.Pitch() * pass_rowstep[pass]);

            for(line = pass_rowoffset[pass] + pass_rowstep[pass];
                line < header.height;
                line += pass_rowstep[pass]) {
                ft = linedata[inpos++];
                if(inpos + passlinelength > maxlen) {
                    pix.UnlockWrite();
                    return 0;
                }
                if(ft < filters.count) {
                    filters.items[ft]->ProcessScanlineR(linedata.data() + inpos, passlinelength
                        , pixelupline_ptr, pixelline_ptr
                        , (header.width * steppixelbytes) - offsetcolbytes
                        , pixelbytes, stepcolbytes, stepcolbytes);
                }
                pixelupline_ptr = pixelline_ptr;
                pixelline_ptr += pix.Pitch() * pass_rowstep[pass];
                inpos += passlinelength;
            }

            // Process indexed color
            if(header.colortype == 3) {
                Deindex(pixel_ptr, pix);
            }

#ifdef PNG_PROGRESSIVE_DISPLAY
            // progressive extrapolation
            pixelupline_ptr = pixel_ptr + (pix.Pitch() * pass_rowoffset[pass]);
            passlinelength  = header.width;
            if(passlinelength >= pass_coloffset[pass]) {
                passlinelength -= pass_coloffset[pass];
            }
            if(pixelsperbyte) {
                if(passlinelength & (pixelsperbyte - 1)) {
                    passlinelength += pixelsperbyte;
                }
                passlinelength /= pixelsperbyte;
            }
            passlinelength *= steppixelbytes;
            stepcolbytes = pass_colstep[pass] * steppixelbytes;
            offsetcolbytes = pass_coloffset[pass] * steppixelbytes;
            for(line = pass_rowoffset[pass]; line < header.height; line += pass_rowstep[pass]) {
                pixelline_ptr = pixelupline_ptr;
                uint32_t pixelcpb;
                for(pixelcpb = offsetcolbytes;
                    pixelcpb + (stepcolbytes) < passlinelength;
                    pixelcpb += stepcolbytes) {
                    for(uint32_t pixelcp = steppixelbytes;
                        pixelcp + pixelcpb < passlinelength && pixelcp < (stepcolbytes - offsetcolbytes);
                        pixelcp += steppixelbytes) {
                        for(uint32_t pxi = 0; pxi < steppixelbytes; pxi++) {
                            pixelline_ptr[pxi + pixelcp + pixelcpb] = pixelupline_ptr[pxi + pixelcpb];
                        }
                    }
                }
                for(uint32_t pixelcp = steppixelbytes;
                    pixelcp + pixelcpb < passlinelength;
                    pixelcp += steppixelbytes) {
                    for(uint32_t pxi = 0; pxi < steppixelbytes; pxi++) {
                        pixelline_ptr[pxi + pixelcp + pixelcpb] = pixelupline_ptr[pxi + pixelcpb];
                    }
                }
                pixelline_ptr += pix.Pitch();
                for(uint32_t lp = pass_rowoffset[pass] + 1;
                    lp < pass_rowstep[pass] && line+lp < header.height;
                    lp++) {
                    uint32_t pixelcpb;
                    for(pixelcpb = offsetcolbytes;
                        pixelcpb + stepcolbytes < passlinelength;
                        pixelcpb += stepcolbytes) {
                        for(uint32_t pixelcp = 0;
                            pixelcp < stepcolbytes - offsetcolbytes;
                            pixelcp ++) {
                            pixelline_ptr[pixelcp+pixelcpb] = pixelupline_ptr[pixelcp+pixelcpb];
                        }
                    }
                    while(pixelcpb < passlinelength) {
                        pixelline_ptr[pixelcpb] = pixelupline_ptr[pixelcpb];
                        pixelcpb++;
                    }
                    pixelline_ptr += pix.Pitch();
                }
                pixelupline_ptr += pix.Pitch() * pass_rowstep[pass];
            }
            pix.Invalidate();
#endif
#ifdef PNG_BEBUG_PASSES
            // XXX: Debugging of passes
            const char * passes[] = {"P1.ppm", "P2.ppm", "P3.ppm", "P4.ppm", "P5.ppm", "P6.ppm", "P7.ppm"};
            pix.PPMDebug(passes[pass]);
#endif
        }
        pix.UnlockWrite();
        return 0;
    }
};

const uint32_t InterlaceTypeAdam7::pass_coloffset[7] = {
    0, 4, 0, 2, 0, 1, 0
};
const uint32_t InterlaceTypeAdam7::pass_colstep[7] = {
    8, 8, 4, 4, 2, 2, 1
};
const uint32_t InterlaceTypeAdam7::pass_rowoffset[7] = {
    0, 0, 4, 0, 2, 0, 1
};
const uint32_t InterlaceTypeAdam7::pass_rowstep[7] = {
    8, 8, 8, 4, 4, 2, 2
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
    std::unique_ptr<InterlaceType> interlace;
    std::unique_ptr<PNGPaletteEntry[]> palette;
    uint32_t palettesize = 0;
    uint32_t paletteread = 0;
    FilterMethodTable filtermethod;
    FilterType filter_null;
    FilterTypeSub filter_sub;
    FilterTypeUp filter_up;
    FilterTypeAverage filter_average;
    FilterTypePaeth filter_paeth;

    filtermethod.count = 5;
    filtermethod.items[0] = &filter_null;
    filtermethod.items[1] = &filter_sub;
    filtermethod.items[2] = &filter_up;
    filtermethod.items[3] = &filter_average;
    filtermethod.items[4] = &filter_paeth;

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
        if(chunk.type == FourCC("IDAT")) {
            if(!idatmode) {
                decoder.DecompressStart();
                idatmode = true;
            }
            util::DataString compresseddata = f.Read(chunk.len);
            chunk.len = 0;
            chunk.crc.Update(compresseddata.data(), compresseddata.length());
            if(decoder.DecompressData(std::move(compresseddata))) {
                if(decoder.ErrorState().error_code != 1) {
                    return decoder.ErrorState();
                }
            }
        }
        else if(chunk.type == FourCC("IEND")) {
            if(interlace && decoder.DecompressHasOutput()) {
                if(header.colortype == 3 && palette) {
                    interlace->AssignPalette(palette.get(), paletteread);
                }
                interlace->Deinterlace(decoder.DecompressGetOutput(), pix);
            }
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
                switch(header.depth) {
                case 1:
                case 2:
                case 4:
                    switch(header.colortype) {
                    case 0:
                        pix.Create(header.width, header.height, 8, ImageColorMode::MONOCHROME);
                        break;
                    case 3:
                        needpalette = true;
                        pix.Create(header.width, header.height, 8, ImageColorMode::COLOR_RGBA);
                        break;
                    default:
                        return void_er(-1, "Invalid color depth for mode");
                        break;
                    }
                    break;
                case 8:
                    switch(header.colortype) {
                    case 0:
                        pix.Create(header.width, header.height, header.depth, ImageColorMode::MONOCHROME);
                        break;
                    case 2:
                        pix.Create(header.width, header.height, header.depth, ImageColorMode::COLOR_RGB);
                        break;
                    case 3:
                        needpalette = true;
                        pix.Create(header.width, header.height, 8, ImageColorMode::COLOR_RGBA);
                        break;
                    case 4:
                        pix.Create(header.width, header.height, header.depth, ImageColorMode::MONOCHROME_A);
                        break;
                    case 6:
                        pix.Create(header.width, header.height, header.depth, ImageColorMode::COLOR_RGBA);
                        break;
                    default:
                        return void_er(-1, "Invalid color depth for mode");
                        break;
                    }
                    break;
                case 16:
                    switch(header.colortype) {
                    case 0:
                        pix.Create(header.width, header.height, header.depth, ImageColorMode::MONOCHROME);
                        break;
                    case 2:
                        pix.Create(header.width, header.height, header.depth, ImageColorMode::COLOR_RGB);
                        break;
                    case 4:
                        pix.Create(header.width, header.height, header.depth, ImageColorMode::MONOCHROME_A);
                        break;
                    case 6:
                        pix.Create(header.width, header.height, header.depth, ImageColorMode::COLOR_RGBA);
                        break;
                    default:
                        return void_er(-1, "Invalid color depth for mode");
                        break;
                    }
                    break;
                default:
                    return void_er(-1, "Invalid color depth");
                    break;
                }
                if(header.filter != 0) {
                    return void_er(-1, "Invalid/unsupported filter method");
                }
                if(header.compression != 0) {
                    return void_er(-1, "Invalid/unsupported compression method");
                }
                if(needpalette) {
                    palettesize = 1 << header.depth;
                    palette = std::unique_ptr<PNGPaletteEntry[]>(new PNGPaletteEntry[palettesize]);
                }
                switch(header.interlace) {
                case 0:
                    pix.meta.push_back(Property("interlace", false));
                    interlace = std::unique_ptr<InterlaceType>(new InterlaceType(filtermethod, header));
                    break;
                case 1:
                    pix.meta.push_back(Property("interlace", true));
                    interlace = std::unique_ptr<InterlaceType>(new InterlaceTypeAdam7(filtermethod, header));
                    break;
                default:
                    return void_er(-1, "Invalid/unsupported interlace method");
                    break;
                }
#ifdef PNG_DEBUG_OUTPUT
                // XXX: debug info
                std::fprintf(stderr, "Image is %d x %d @%d\n", header.width, header.height, header.depth);
                std::fprintf(stderr, "Color: %d\nCompressor: %d\nFilter: %d\nInterlace: %d\n",
                        header.colortype, header.compression, header.filter, header.interlace);
#endif
                needheader = false;
            } else {
                return void_er(-1, "Multiple header chunks");
            }
        }
        else if(chunk.type == FourCC("PLTE")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple PLTE chunk");
            }
            if(!palettesize) {
                palettesize = 256;
                palette = std::unique_ptr<PNGPaletteEntry[]>(new PNGPaletteEntry[palettesize]);
            }
            while(chunk.len > 0 && !chunk.End() && paletteread < palettesize) {
                chunk >> palette[paletteread].red;
                if(!chunk.len) {
                    return void_er(-1, "Invalid PLTE chunk");
                }
                chunk >> palette[paletteread].green;
                if(!chunk.len) {
                    return void_er(-1, "Invalid PLTE chunk");
                }
                chunk >> palette[paletteread].blue;
                palette[paletteread].alpha = 0xff;
                paletteread++;
            }
        }
        else if(chunk.type == FourCC("gAMA")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple gAMA chunk");
            }
            chunk >> gama;
            pix.meta.push_back(Property("gama", (uint32_t)gama));
#ifdef PNG_DEBUG_OUTPUT
            std::cerr << "Gama: " << gama << '\n';
#endif
            chunkcounts[chunk.type.ldata] = 1;
        }
        else if(chunk.type == FourCC("tIME")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple tIME chunk");
            }
            chunkcounts[chunk.type.ldata] = 1;

            chunk >> mtime.year >> mtime.month >> mtime.day;
            chunk >> mtime.hour >> mtime.minute >> mtime.second;
            pix.meta.push_back(Property("modified", mtime));
#ifdef PNG_DEBUG_OUTPUT
            // XXX: debug info
            std::fprintf(stderr, "Modification: %d:%d:%d %dD %dM %dY\n"
                , mtime.hour, mtime.minute, mtime.second, mtime.day, mtime.month, mtime.year);
#endif
        }
        else if(chunk.type == FourCC("bKGD")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple bKGD chunk");
            }
            chunkcounts[chunk.type.ldata] = 1;

            background.type = header.colortype;
            chunk >> background;
            pix.meta.push_back(Property("background", background));
#ifdef PNG_DEBUG_OUTPUT
            // XXX: debug info
            if(background.type == 0 || background.type == 4) {
                std::fprintf(stderr, "Background: Y %d\n", background.value);
            }
            if(background.type == 2 || background.type == 6) {
                std::fprintf(stderr, "Background: RGB %d %d %d\n"
                    , background.red, background.green, background.blue);
            }
            if(background.type == 3) {
                std::fprintf(stderr, "Background: I %d\n", background.index);
            }
#endif
        }
        else if(chunk.type == FourCC("pHYs")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple pHYs chunk");
            }
            chunkcounts[chunk.type.ldata] = 1;
            PNGLong pix_x, pix_y;
            uint8_t unit;
            chunk >> pix_x >> pix_y >> unit;
            pix.meta.push_back(Property("scale-x", (uint32_t)pix_x));
            pix.meta.push_back(Property("scale-y", (uint32_t)pix_y));
#ifdef PNG_DEBUG_OUTPUT
            // XXX: debug info
            std::fprintf(stderr, "Pixelsize: %d x %d ", pix_x, pix_y);
            if(unit == 1) {
                std::fprintf(stderr, "pixels/meter\n");
            }
            else {
                std::fprintf(stderr, "unknown\n");
            }
#endif
        }
        else if(chunk.type == FourCC("tRNS")) {
            if(chunkcounts.count(chunk.type.ldata)) {
                return void_er(-1, "Multiple tRNS chunk");
            }
            chunkcounts[chunk.type.ldata] = 1;

            trans.type = header.colortype;
            if(header.colortype == 3 && palette) {
                uint32_t tindex = 0;
                while(chunk.len && !chunk.End() && tindex < palettesize) {
                    chunk >> palette[tindex++].alpha;
                }
            }
            else {
                chunk >> trans;
            }
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
            // TODO: something with the text, log it maybe
            pix.meta.push_back(Property(keyword, textdata));
#ifdef PNG_DEBUG_OUTPUT
            std::cerr << "Keyword: \"" << keyword << "\" = \"" << textdata << "\"\n";
#endif
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
            // TODO: something with the text, log it maybe

            std::string stringdata((const char*)textdata.data(), textdata.length());
            pix.meta.push_back(Property(keyword, stringdata));
#ifdef PNG_DEBUG_OUTPUT
            std::cerr << "zKeyword: \"" << keyword << "\" = \"";
            std::cerr.write((const char*)textdata.data(), textdata.length());
            std::cerr << "\"\n";
#endif
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
#ifdef PNG_DEBUG_OUTPUT
            std::cerr << inlong << "!=" << chunk.crc.ldata << '\n';
#endif
            return void_er(-1, "CRC Failure");
        }
    }
    return stat;
}

} // png
} // resource
} // trillek
