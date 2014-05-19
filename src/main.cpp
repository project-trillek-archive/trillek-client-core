#include "OS.h"
#include "systems/json-parser.hpp"

int main(int argCount, char **argValues) {
    trillek::OS os;
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);

    trillek::system::JSONParser jparser;
    jparser.Parse("assets/tests/sample.json");

    while (!os.Closing()) {
        os.OSMessageLoop();
    }

    os.Terminate();

    return 0;
}
