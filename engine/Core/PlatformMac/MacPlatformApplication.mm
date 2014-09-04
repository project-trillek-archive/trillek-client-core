//=============================================================================

#include "Core.h"

#include <mach-o/dyld.h>
#include <unistd.h>

using namespace trillek::core::platform;


//-----------------------------------------------------------------------------

MacApplicationRef MacApplication::SharedApplication()
{
    static std::unique_ptr<ThisClass> Instance;
    static std::once_flag OnceFlag;
    
    std::call_once(OnceFlag, []() {
        Instance.reset( new ThisClass() );
    });
    
    return Instance;
}


//-----------------------------------------------------------------------------
#pragma mark -  Acessing The Runtime Environment
//-----------------------------------------------------------------------------

// Dummy class to use as the bundle target.
@interface TCoreMacApplicationDummyClass : NSObject @end
@implementation TCoreMacApplicationDummyClass @end


void MacApplication::GetExecutablePath(std::string &result)
{
    uint32_t resultLen = 0;
    _NSGetExecutablePath(NULL, &resultLen);
    
    char buffer[resultLen];
    _NSGetExecutablePath(buffer, &resultLen);
    
    result = buffer;
}


void MacApplication::GetResourcesDirectory(std::string &result)
{
    @autoreleasepool {
        NSBundle *currentBundle = [NSBundle bundleForClass:TCoreMacApplicationDummyClass.class];
        if (currentBundle) {
            NSString *resourcesPath = [currentBundle resourcePath];
            result = [resourcesPath cStringUsingEncoding:NSASCIIStringEncoding];
        } else
            GetWorkingDirectory(result);
    }
}


void MacApplication::GetWorkingDirectory(std::string &result)
{
    char buffer[PLATFORM_MAX_PATH];
    result = getcwd(buffer, PLATFORM_MAX_PATH);
}


void MacApplication::SetWorkingDirectory(std::string &newWorkingDirectory)
{
    chdir(newWorkingDirectory.c_str());
}
