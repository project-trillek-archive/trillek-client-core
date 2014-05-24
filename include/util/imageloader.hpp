#ifndef IMAGELOADER_HPP_INCLUDED
#define IMAGELOADER_HPP_INCLUDED

#include "util/utiltype.hpp"
#include "resources/pixel-buffer.hpp"
#include <istream>

namespace trillek {
namespace resource {
namespace png {
util::void_er Load(util::InputStream & f, PixelBuffer & pix);
}

} // resource
} // trillek

#endif
