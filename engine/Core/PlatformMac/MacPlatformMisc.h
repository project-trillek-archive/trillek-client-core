//=============================================================================
// OS X specialization of CommonPlatformMisc.h
//=============================================================================

namespace trillek { namespace core { namespace platform {

    /* OS X specialization of BaseMisc */
    class MacMisc : public Misc {
        static const char* GetPlatformName()
        { return "Mac"; }
        static uint16_t GetNumberOfCores();
        static void GetEnvironmentVariable(const char *name, std::string &result);
        
        FORCEINLINE static void CachePrefetchR(const void *addr, size_t numBytes)
        { __builtin_prefetch(addr, 0, 0); }
        FORCEINLINE static void CachePrefetchW(const void *addr, size_t numBytes)
        { __builtin_prefetch(addr, 1, 0); }
    };
    
}}}


namespace trillek { namespace core {
    typedef platform::MacMisc PlatformMisc;
}}