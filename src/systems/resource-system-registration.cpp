#include "systems/ResourceSystem.h"

#include "resources/Shader.hpp"
#include "resources/MD5Mesh.hpp"
#include "resources/TextFile.h"

namespace trillek {
namespace system {

void ResourceSystem::RegisterTypes() {
    Register<trillek::resource::TextFile>();
    Register<trillek::resource::MD5Mesh>();
    Register<trillek::resource::Shader>();
}

} // End of system
} // End of trillek
