//=============================================================================
//! \ingroup CORE
//! \file   PlatformClasses.h
//! \brief  Platform specific includes specific to the trillek engine.
//!
//! \addtogroup PLATFORM
//! \{
//=============================================================================

// NOTE: System includes do not belong here.  Put them in CommonPlatform.h or
//       <PlatformName>Platform.h.

#include "PlatformCommon/CommonPlatformMisc.h"
#include "PlatformCommon/CommonPlatformApplication.h"
#include "PlatformCommon/CommonPlatformWindow.h"

// Platform specific includes
#if PLATFORM_MAC
    #include "PlatformMac/MacPlatformClasses.h"
#endif


//=============================================================================
//! \}
