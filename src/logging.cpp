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
    std::string msgline("[");
    msgline.append(l).append("] ",2);
    msgline.append(message);
    std::unique_lock<std::mutex>(log_m);
    std::cerr << msgline << '\n';
}
void Logging::WriteLine(std::string l, std::string section, std::string message) {
    std::string msgline("[");
    msgline.append(l).append("-",1).append(section).append("] ",2);
    msgline.append(message);
    std::unique_lock<std::mutex>(log_m);
    std::cerr << msgline << '\n';
}

} // namespace trillek
