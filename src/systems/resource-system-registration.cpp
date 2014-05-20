#include "systems/ResourceSystem.h"

#include "resources/Shader.hpp"
#include "resources/MD5Mesh.hpp"
#include "resources/TextFile.h"

namespace trillek {
namespace system {

void ResourceSystem::RegisterTypes() {
    RegisterResourceType<trillek::resource::TextFile>();
    RegisterResourceType<trillek::resource::MD5Mesh>();
    RegisterResourceType<trillek::resource::Shader>();
}

} // End of system
} // End of trillek
