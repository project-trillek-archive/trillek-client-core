//=============================================================================
// OS X specialization of CommonPlatformWindow.h
//=============================================================================

namespace trillek { namespace core { namespace platform {
   
    /* OS X specialization of Window */
    CORE_API(MacWindow)
    class MacWindow : public Window {
        DECLARE_CLASS(MacWindow, Window)
        
    public:
        MacWindow(const glm::vec2 size, const std::string &title);
    };
    
}}}

namespace trillek { namespace core {
    typedef platform::MacWindow PlatformWindow;
    REEXPORT_API(platform::MacWindow, PlatformWindow);
}}
