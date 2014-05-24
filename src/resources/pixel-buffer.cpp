
#include "util/utiltype.hpp"
#include "util/imageloader.hpp"
#include "resources/pixel-buffer.hpp"

namespace trillek {
namespace resource {

PixelBuffer::PixelBuffer() :
    imagewidth(0), imageheight(0), bufferpitch(0), imagebitdepth(0),
    imagemode(ImageColorMode::MONOCHROME), imagepixelsize(0),
    image_x(0), image_y(0), dirty(false)
{
}

PixelBuffer::~PixelBuffer() {

}

PixelBuffer::PixelBuffer(PixelBuffer && rv) {
    writelock.lock();
    rv.writelock.lock();
    imagewidth = rv.imagewidth;
    imageheight = rv.imageheight;
    bufferpitch = rv.bufferpitch;
    imagebitdepth = rv.imagebitdepth;
    imagepixelsize = rv.imagepixelsize;
    imagemode = rv.imagemode;
    image_x = rv.image_x;
    image_y = rv.image_y;
    dirty = rv.dirty;
    blockptr = std::move(rv.blockptr);
    rv.imageheight = 0;
    rv.imagewidth = 0;
    rv.bufferpitch = 0;
    rv.writelock.unlock();
    writelock.unlock();
}
PixelBuffer::PixelBuffer(uint32_t width, uint32_t height, uint32_t bitspersample, ImageColorMode mode) {
    imagepixelsize = 0;
    imagewidth = 0;
    imageheight = 0;
    imagebitdepth = 0;
    imagemode = ImageColorMode::MONOCHROME;
    image_x = 0;
    image_y = 0;
    dirty = false;
    Create(width, height, bitspersample, mode);
}

PixelBuffer & PixelBuffer::operator=(PixelBuffer && rv) {
    writelock.lock();
    rv.writelock.lock();
    imagewidth = rv.imagewidth;
    imageheight = rv.imageheight;
    bufferpitch = rv.bufferpitch;
    imagebitdepth = rv.imagebitdepth;
    imagepixelsize = rv.imagepixelsize;
    imagemode = rv.imagemode;
    image_x = rv.image_x;
    image_y = rv.image_y;
    dirty = rv.dirty;
    blockptr = std::move(rv.blockptr);
    rv.imageheight = 0;
    rv.imagewidth = 0;
    rv.bufferpitch = 0;
    rv.writelock.unlock();
    writelock.unlock();
    return *this;
}

bool PixelBuffer::Create(uint32_t width, uint32_t height, uint32_t bitspersample, ImageColorMode mode) {
    uint8_t pixelsize;
    switch(mode) {
    case ImageColorMode::COLOR_RGB:
        pixelsize = 3;
        break;
    case ImageColorMode::COLOR_RGBA:
        pixelsize = 4;
        break;
    case ImageColorMode::MONOCHROME:
        pixelsize = 1;
        break;
    case ImageColorMode::MONOCHROME_A:
        pixelsize = 2;
        break;
    default:
        pixelsize = 0;
        break;
    }
    imagepixelsize = pixelsize;
    imagewidth = width;
    imageheight = height;
    imagebitdepth = bitspersample;
    imagemode = mode;
    image_x = 0;
    image_y = 0;
    dirty = true;
    bufferpitch = (width * bitspersample * imagepixelsize);
    bufferpitch = (bufferpitch >> 3) + ((bufferpitch & 0x7) ? 1 : 0);

    blockptr = std::unique_ptr<uint8_t[]>(new uint8_t[bufferpitch * height]);
    return true;
}

bool PixelBuffer::IsDirty() const {
    return dirty;
}
void PixelBuffer::Invalidate() {
    dirty = true;
}
void PixelBuffer::Validate() {
    dirty = false;
}

bool PixelBuffer::Initialize(const std::vector<Property> &properties) {
    std::string fname;
    for(const Property& p : properties) {
        std::string name = p.GetName();
        if(name == "filename") {
            fname = p.Get<std::string>();
        }
    }

    std::ifstream f(fname, std::ios::in | std::ios::binary);

    if(f.is_open()) {
        util::StdInputStream insf(f);
        png::Load(insf, *this);
        f.close();

        return true;
    }

    return false;
}

}
} // trillek
