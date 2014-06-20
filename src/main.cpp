#include "os.hpp"
#include "systems/json-parser.hpp"
#include "systems/transform-system.hpp"
#include "systems/resource-system.hpp"
#include "systems/graphics.hpp"
#include "systems/sound-system.hpp"
#include <cstddef>

size_t gAllocatedSize = 0;

int main(int argCount, char** argValues) {
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
    //jparser.Parse("assets/tests/sample.json");
    jparser.Parse("assets/settings/audio.json");

    std::shared_ptr<trillek::sound::System> soundsystem = trillek::sound::System::GetInstance();
    soundsystem->SetListenerPosition(glm::vec3(0, 0, 0));
    soundsystem->SetListenerOrientation(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

    std::shared_ptr<trillek::sound::Sound> s1 = soundsystem->GetSound("music_track_1");
    s1->Play();

    trillek::graphics::System gl;
    gl.Start(os.GetWindowWidth(), os.GetWindowHeight());

    double x, y = 0.0;
    double time = 0.0;

    while(!os.Closing()) {
        os.OSMessageLoop();

        // sound
        {
            soundsystem->Update();
            // sound position
            const double dt = os.GetDeltaTime();
            time += dt;
            x = cos(time) * 2.0;
            y = sin(time) * 2.0;
            s1->SetPosition(glm::vec3(x, 0, y));
        }

        gl.Update(0);
        os.SwapBuffers();
    }

    os.Terminate();

    return 0;
}
