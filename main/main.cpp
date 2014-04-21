#include <iostream>

int main(int argCount, char **argValues) {
    #ifdef STANDALONE
    std::cout << "Starting Trillek with standalone build..." << std::endl;
    #endif // STANDALONE
	return 0;
}
