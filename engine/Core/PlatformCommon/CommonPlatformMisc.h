//=============================================================================
//! \ingroup CORE
//! \file   CommonPlatformMisc.h
//! \brief  Generic platform class to abstract misc functionality.
//=============================================================================

namespace trillek { namespace core { namespace platform {
    
    /**
     *  Abstracts various bits of the current platform that don't deserve their
     *  own header file.
     *
     *  \note
     *  Do not use this class directly!  Use \c PlatformMisc which is
     *  typedef'd to the platform specific subclass.
     */
    class Misc {
    public:
        
        static const char* GetPlatformName()
        { return ""; }
        
        static uint16_t GetNumberOfCores()
        { return 1; }
        
        static void GetEnvironmentVariable(const char *name, std::string &result)
        { }
        
        /** Move \a numBytes of data at \a addr into the cache. */
        FORCEINLINE static void CachePrefetchR(const void *addr, size_t numBytes)
        { }
        /** Move \a numBytes of data at \a addr into the cache. */
        FORCEINLINE static void CachePrefetchW(const void *addr, size_t numBytes)
        { }
        
    };
    
}}}



namespace trillek { namespace core {
#ifdef __doxygen__
    /** Typedef'd to the correct specialization of \c platform::Misc for
     *  the current platform. */
    typedef void PlatformMisc;
#endif
}}
