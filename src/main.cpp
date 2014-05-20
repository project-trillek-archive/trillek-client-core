#include "OS.h"

int main(int argCount, char **argValues) {
    trillek::OS os;
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);

    while (!os.Closing()) {
        os.OSMessageLoop();
    }

    os.Terminate();
    return 0;
}
