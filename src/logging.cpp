#include "logging.hpp"

#include <iostream>

namespace trillek {

std::unique_ptr<Logging> Logging::instance;

Logging::Logging() {

}

Logging& Logging::GetInstance() {
    if(!instance) {
        instance.reset(new Logging());
    }
    return *instance.get();
}
void Logging::WriteLine(std::string l, std::string message) {
    std::cerr << "[" << l << "] " << message << '\n';
}
void Logging::WriteLine(std::string l, std::string section, std::string message) {
    std::cerr << "[" << l << "-" << section << "] " << message << '\n';
}

} // namespace trillek
