#include "resources/shader.hpp"
#include "resources/md5mesh.hpp"
#include "resources/pixel-buffer.hpp"
#include "resources/text-file.hpp"

namespace trillek {
namespace resource {

void System::RegisterTypes() {
    RegisterResourceType<trillek::resource::TextFile>();
    RegisterResourceType<trillek::resource::MD5Mesh>();
    RegisterResourceType<trillek::resource::Shader>();
    RegisterResourceType<trillek::resource::PixelBuffer>();
}

} // End of resource
} // End of trillek
