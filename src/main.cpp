#include "OS.h"
#include "systems/json-parser.hpp"
#include "systems/transform-system.hpp"
#include "systems/ResourceSystem.h"

int main(int argCount, char **argValues) {
    trillek::OS os;
#if __APPLE__
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 2);
#else
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);
#endif

    trillek::system::TransformSystem::GetInstance();
    trillek::system::ResourceSystem::GetInstance();

    trillek::system::JSONParser jparser;
    jparser.Parse("assets/tests/sample.json");

    trillek::graphics::GL gl;
    gl.Start(os.GetWindowWidth(), os.GetWindowHeight());
    while (!os.Closing()) {
        os.OSMessageLoop();
        gl.Update(0);
        os.SwapBuffers();
    }

    os.Terminate();

    return 0;
}
