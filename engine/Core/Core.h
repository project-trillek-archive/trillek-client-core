//=============================================================================
//! \file   Core.h
//! \brief  Root include for the Core module
//
#ifndef CORE_H
#define CORE_H
//
//! \defgroup CORE Core
//! Files pertinent to all parts of the engine.  This module depends on no
//! other and most other modules will depend on it.  Contains platform and
//! compiler abstractions. It includes most of the system headers needed by
//! the rest of the engine.
//
//! @namespace trillek::core
//! \ingroup CORE
//! \brief Namespace for the Core module.
//=============================================================================
    
//=============================================================================
//! \ingroup CORE
//! \defgroup BUILD Build
//! Macros that control the build.
//=============================================================================

#include "Build.h"


//=============================================================================
//! \ingroup CORE
//! \defgroup PLATFORM Platform
//! Platform detection and abstraction.
//
//! @namespace trillek::core::platform
//! \ingroup PLATFORM
//! \brief Namespace for the \ref PLATFORM group of the Core module.
//=============================================================================

#include "Platform.h"


//=============================================================================
//! \ingroup CORE
//! \defgroup TEMPLATES Templates
//! Useful generic templates.
//=============================================================================



#include "MetaMacros.h"
#include "ClassBase.h"
#include "PlatformClasses.h"


//=============================================================================
#endif