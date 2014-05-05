//=============================================================================

#include "Core.h"

#include <sys/sysctl.h>

using namespace trillek::core::platform;


uint16_t MacMisc::GetNumberOfCores()
{
    static uint16_t NumberOfCores = 0xFFFF;
    if (UNLIKELY(NumberOfCores == 0xFFFF)) {
        size_t uint16size = sizeof(NumberOfCores);
        
        if (sysctlbyname("hw.physicalcpu", &NumberOfCores, &uint16size, NULL, 0))
            NumberOfCores = 1; // Error
    }
    return NumberOfCores;
}


void MacMisc::GetEnvironmentVariable(const char *name, std::string &result)
{
    result = getenv(name);
}
