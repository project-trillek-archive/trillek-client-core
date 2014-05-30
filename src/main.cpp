#include "os.hpp"
#include "systems/json-parser.hpp"
#include "systems/transform-system.hpp"
#include "systems/resource-system.hpp"
#include "systems/graphics.hpp"
#include <cstddef>

size_t gAllocatedSize = 0;

int main(int argCount, char **argValues) {
    trillek::OS os;
#if __APPLE__
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 2);
#else
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);
#endif

    // Call each system's GetInstance to create the initial instance.
    trillek::transform::System::GetInstance();
    trillek::resource::System::GetInstance();

    trillek::json::System jparser;
    jparser.Parse("assets/tests/sample.json");


    trillek::graphics::System gl;
    gl.Start(os.GetWindowWidth(), os.GetWindowHeight());

    while (!os.Closing()) {
        os.OSMessageLoop();
        gl.Update(0);
        os.SwapBuffers();
    }

    os.Terminate();

    return 0;
}
