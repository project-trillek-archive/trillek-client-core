#include "OS.h"

int main(int argCount, char **argValues) {
    trillek::OS os;
#if __APPLE__
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 2);
#else
    os.InitializeWindow(800, 600, "Trillek Client Core", 3, 0);
#endif

    while (!os.Closing()) {
        os.OSMessageLoop();
    }

    os.Terminate();
	return 0;
}
