//=============================================================================

#include "Core.h"

using namespace trillek::core::assertion;


void Handler::AssertionFailed(const char *expression, const char *file, int line, const char *message, ...)
{
    va_list ap;
    va_start (ap, message);
    
    char messageBuffer[4096];
    vsprintf(messageBuffer, message, ap);
    va_end(ap);
    
    char exceptionString[5048];
    sprintf(exceptionString, "Assertion failed: %s [%s line %i]: %s", expression, file, line, messageBuffer);
    
    throw new std::runtime_error(exceptionString);
}
