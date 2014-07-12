#include <iostream>

int main(int argCount, char **argValues) {
    #ifdef SERVER
    std::cout << "Starting Trillek server..." << std::endl;
    #endif // SERVER
	return 0;
}
