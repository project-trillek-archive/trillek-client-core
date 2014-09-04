//=============================================================================
// OS X specialization of CommonPlatformApplication.h
//=============================================================================

namespace trillek { namespace core { namespace platform {

    /* OS X specialization of Application */
    CORE_API(MacApplication)
    class MacApplication : public Application {
        DECLARE_CLASS(MacApplication, Application)
        using Application::Application;
        
    public:
        static MacApplicationRef SharedApplication();
        
    public:
        void GetExecutablePath(std::string &result);
        void GetResourcesDirectory(std::string &result);
        void GetWorkingDirectory(std::string &result);
        void SetWorkingDirectory(std::string &newWorkingDirectory);
    };
    
}}}



namespace trillek { namespace core {
    typedef platform::MacApplication PlatformApplication;
    REEXPORT_API(platform::MacApplication, PlatformApplication);
}}
