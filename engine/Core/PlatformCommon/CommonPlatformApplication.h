//=============================================================================
//! \ingroup CORE
//! \file   CommonPlatformApplication.h
//=============================================================================

namespace trillek { namespace core { namespace platform {
    
    /** 
     *  Represents the currently running application. 
     *
     *  \note
     *  Do not use this class directly!  Use \c PlatformApplication which is
     *  typedef'd to the platform specific subclass.
     */
    CORE_API(Application)
    class Application {
        DECLARE_CLASS(Application)
        
        // Singleton
        Application();
        Application(const Application&);
        
    //-------------------------------------------------------------------------
    #pragma mark    Retrieving The Shared Application
    //! \name       Retrieving The Shared Application
    //-------------------------------------------------------------------------
    public:
        /** Returns the shared instance representing the currently running
         *  application. */
    #ifdef __doxygen__
        static PlatformApplicationRef SharedApplication();
    #else
        static ApplicationRef SharedApplication();
    #endif
        
    //-------------------------------------------------------------------------
    #pragma mark    Acessing The Runtime Environment
    //! \name       Acessing The Runtime Environment
    //-------------------------------------------------------------------------
    public:
        /** Returns the full path to the executable, including the executable
         *  name. */
        void GetExecutablePath(std::string &result)
        { }
        
        /** Returns the path to the application's resources folder. */
        void GetResourcesDirectory(std::string &result)
        { }
        
        /** Returns the current working directory. */
        void GetWorkingDirectory(std::string &result)
        { }
        /** Changes the current working directory */
        void SetWorkingDirectory(std::string &newWorkingDirectory)
        { }
        
    //-------------------------------------------------------------------------
    #pragma mark    Message Loop
    //! \name       Message Loop
    //-------------------------------------------------------------------------
    public:
        /** Dispatch any awaing messages from the OS. */
        void MessageLoop();
        
    };
    
}}}



namespace trillek { namespace core {
#ifdef __doxygen__
    /** Typedef'd to the correct specialization of
     *  \c platform::CommonApplication for the current platform. */
    typedef void PlatformApplication;
#endif
}}
