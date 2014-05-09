
#include "util/Compression.hpp"

namespace trillek {
namespace util {
namespace algorithm {

Inflate::Inflate() {
}
Inflate::~Inflate() {
}
bool Inflate::DecompressStart() {
    return false;
}
bool Inflate::DecompressEnd() {
    return false;
}
bool Inflate::DecompressData(std::string) {
    return false;
}
bool Inflate::DecompressHasOutput() {
    return false;
}
std::string Inflate::DecompressGetOutput() {
    return std::string();
}

} // algorithm
} // util
} // trillek
