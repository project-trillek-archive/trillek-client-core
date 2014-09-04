//=============================================================================
//! \ingroup CORE
//! \file   CommonPlatform.h
//!
//! \addtogroup PLATFORM
//! \{
//=============================================================================

//-----------------------------------------------------------------------------
#pragma mark -  Includes
//-----------------------------------------------------------------------------

#include <new>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <iostream>
#include <stdexcept>
#include <mutex>

#include <glm/glm.hpp>

// Anything included here must be present on all platforms.  Platform specific
// includes go in <PlatformName>Platform.h.


//=============================================================================
//! \}