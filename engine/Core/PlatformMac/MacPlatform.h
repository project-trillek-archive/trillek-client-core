//=============================================================================
// OS X Platform
//=============================================================================


//-----------------------------------------------------------------------------
#pragma mark -  Includes
//-----------------------------------------------------------------------------

#include <sys/syslimits.h>
#include <pthread.h>

#include <CoreFoundation/CoreFoundation.h>
#ifdef __OBJC__
    #include <Cocoa/Cocoa.h>
#endif

// OS X defines these in one of the system headers.
#undef check
#undef verify
#undef require

//-----------------------------------------------------------------------------

#define PLATFORM_64BITS __LP64__

// Compiler Features
#if  __has_feature(cxx_exceptions)
#   define PLATFORM_EXCEPTIONS_DISABLED 0
#else
#   define PLATFORM_EXCEPTIONS_DISABLED 1
#endif


// Platform Attributes
#define PLATFORM_MAX_PATH   PATH_MAX
#define PLATFORM_HAS_DTRACE 1


// Function Modifiers
#define FORCEINLINE inline __attribute__((__always_inline__))
#define FORCENOINLINE __attribute__((__noinline__))


// Linker
#define DLLIMPORT extern


// Compiler Intrinsics
#define LIKELY(...) __builtin_expect((##__VA_ARGS__), 1)

// Cache Hinting
#define cache_prefetch_r(ADDR, LOCALITY) __builtin_prefetch(#ADDR, 0, #LOCALITY)
#define cache_prefetch_w(ADDR, LOCALITY) __builtin_prefetch(#ADDR, 1, #LOCALITY)
