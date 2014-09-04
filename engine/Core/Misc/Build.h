//=============================================================================
//! \ingroup CORE
//! \file   Build.h
//! \brief  Macros that control the build.
//!
//! \addtogroup BUILD
//! \{
//=============================================================================

//-----------------------------------------------------------------------------
#pragma mark -  Build Configuration
//! \name       Build Configuration
//!
//! \brief      These should be defined by the compiler flags.
//-----------------------------------------------------------------------------

#ifndef BUILD_DEBUG
    #define BUILD_DEBUG (!DNDEBUG)
#endif

/** Building on a developer's machine. */
#ifndef BUILD_DEVELOPMENT
    #define BUILD_DEVELOPMENT 0
#endif

/** Building for running unit tests. */
#ifndef BUILD_TEST
    #define BUILD_TEST 0
#endif

/** Building for profiiling. */
#ifndef BUILD_PROFILE
    #define BUILD_PROFILE 0
#endif

/** Building to run on an end user's machine */
#ifndef BUILD_SHIPPING
    #define BUILD_SHIPPING (DNDEBUG)
#endif

// The above options are all multually exclusive and 1 musy be enabled.
#if BUILD_DEBUG + BUILD_DEVELOPMENT + BUILD_TEST + BUILD_PROFILE + BUILD_SHIPPING != 1
    #error Only 1 build configuration can be active.
#endif


//-----------------------------------------------------------------------------
#pragma mark -  Build Features
//! \name       Build Features
//!
//! \brief      These should be defined by the compiler flags.
//-----------------------------------------------------------------------------

/** This build will display graphics. */
#ifndef BUILD_USES_GRAPHICS
    #define BUILD_USES_GRAPHICS 1 // Default to 0 in the future.
#endif


//-----------------------------------------------------------------------------
#pragma mark -  Build Options
//! \name       Build Options
//-----------------------------------------------------------------------------

/** If true, the \c check assertion macros will be compiled into the
 *  executable. */
#ifndef ENABLE_CHECK
    #define ENABLE_CHECK (BUILD_DEBUG || BUILD_DEVELOPMENT || BUILD_TEST)
#endif

// If you need to include a check or enable take action depending upon
// the type of build, try and add an enabling macro here instead of
// looking at \c BUILD_DEBUG, \c BUILD_PROFILE directly. */

    
//=============================================================================
//! \}