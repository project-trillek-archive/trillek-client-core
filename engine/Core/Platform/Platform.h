//=============================================================================
//! \ingroup CORE
//! \file   Platform.h
//! \brief  Platform detection.
//!
//! \addtogroup PLATFORM
//! \{
//=============================================================================

#include "PlatformCommon/CommonPlatform.h"

//-----------------------------------------------------------------------------
#pragma mark -  Detecting The Current Platform
//-----------------------------------------------------------------------------

// Define all platforms to 0
#define PLATFORM_WINDOWS 0
#define PALTFORM_MAC 0
#define PLATFORM_LINUX 0

#if __APPLE__
    #include <TargetConditionals.h> // To determine if this is Mac or iOS
#endif

// Platform Include
#if TARGET_OS_MAC && !TARGET_OS_EMBEDDED
    #define PLATFORM_MAC 1
    #include "PlatformMac/MacPlatform.h"
#else
    #error Unknown Platform
#endif


// Required platform defines.
#ifndef PLATFORM_64BITS
    #error Please define PLATFORM_64BITS in your platform's header file.
#endif


//-----------------------------------------------------------------------------
#pragma mark -  Compiler Features
//! \name       Compiler Features
//-----------------------------------------------------------------------------

/** Building with exceptions disabled. */
#ifndef PLATFORM_EXCEPTIONS_ENABLED
    #define PLATFORM_EXCEPTIONS_ENABLED 0
#endif

// If we need to know whether the current compiler has a feature, add a
// macro for it here.


//-----------------------------------------------------------------------------
#pragma mark -  Platform Attributes
//! \name       Platform Attributes
//-----------------------------------------------------------------------------

#define PLATFORM_32BITS (!PLATFORM_64BITS)

/** Expected size in bytes of a cache line. */
#ifndef CACHE_LINE_SIZE
    #define CACHE_LINE_SIZE 128
#endif

/** The maximum length (in bytes) of a file path. */
#ifndef PLATFORM_MAX_PATH
    #define PLATFORM_MAX_PATH 1024
#endif

#ifndef PLATFORM_HAS_DTRACE
    #define PLATFORM_HAS_DTRACE 0
#endif

// If we need to know whether the current platform has a feature, add a
// macro for it here.


//-----------------------------------------------------------------------------
#pragma mark -  Attributes and Modifiers
//! \name       Attributes and Modifiers
//-----------------------------------------------------------------------------

#ifndef FORCEINLINE
    #define FORCEINLINE
#endif
#ifndef FORCENOINLINE
    #define FORCENOINLINE
#endif

#ifndef abstract
    #define abstract =0
#endif

#ifndef DLLIMPORT
    #define DLLIMPORT
#endif


//-----------------------------------------------------------------------------
#pragma mark -  Compiler Intrinsics
//! \name       Compiler Intrinsics
//-----------------------------------------------------------------------------

/** Hint that the expression is likely to be \c true. */
#ifndef LIKELY
    #define LIKELY(...) __VA_ARGS__
#endif
/** Hint that the expression is likely to be \c false. */
#ifndef UNLIKELY
    #define UNLIKELY(...) __VA_ARGS__
#endif

// Add useful functions/macros that may be compiler dependent here.


//=============================================================================
//! \}
