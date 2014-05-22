
#include "util/utiltype.hpp"
#include "util/imageloader.hpp"
#include "resources/pixel-buffer.hpp"

namespace trillek {
namespace resource {

PixelBuffer::PixelBuffer() :
    imagewidth(0), imageheight(0), bufferpitch(0), imagebitdepth(0),
    imagemode(ImageColorMode::MONOCHROME),
    image_x(0), image_y(0), dirty(false)
{
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
        LoadPNG(insf, *this);
        f.close();

        return true;
    }

    return false;
}

}
}
