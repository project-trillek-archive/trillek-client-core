#include <iostream>

int main(int argCount, char **argValues) {
    #ifdef CLIENT
    std::cout << "Starting Trillek client..." << std::endl;
    #endif // CLIENT
	return 0;
}
