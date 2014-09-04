//=============================================================================
//! \ingroup CORE
//! \file   CommonPlatformWindow.h
//=============================================================================

namespace trillek { namespace core { namespace platform {

    /**
     *  The \c CommonWindow class represents a native window.
     *
     *  \note
     *  Do not use this class directly!  Use \c PlatformWindow which is
     *  typedef'd to the platform specific subclass.
     *  \note
     *  All functions in this class must be called on the main thread.
     */
    CORE_API(Window)
    class Window {
        DECLARE_CLASS(Window)
        
    //-------------------------------------------------------------------------
    #pragma mark    Creating Windows
    //! \name       Creating Windows
    //-------------------------------------------------------------------------
    public:
        ~Window();
        /** Creates and returns a new window. */
        Window(const glm::vec2 size, const std::string &title);
        
        
        /*template<class ...Args> static
        WindowPtr Create(Args&&... args) {
            return std::unique_ptr<ThisClass>(new ThisClass(std::forward<Args>(args)...));
        }*/
        
    //-------------------------------------------------------------------------
    #pragma mark    Configuring Windows
    //! \name       Configuring Windows
    //-------------------------------------------------------------------------
    public:
        /** Sets the title of this window. */
        void setTitle(const std::string &title);
        
        /** Retrieves the position, in screen coordinates, of the upper-left 
         *  corner of the client area of the specified window. */
        glm::vec2 getPosition();
        /** Sets the position, in screen coordinates, of this window. */
        void setPosition(const glm::vec2 newPosition);
        
        /** Retrieves the size, in screen coordinates, of this window. */
        glm::vec2 getSize();
        /** Sets the size, in screen coordinates, of this window. */
        void setSize(const glm::vec2 newSize);
        
        /** Hides the window. */
        void hide();
        /** Makes the window visible. */
        void show();
       
        
    //-------------------------------------------------------------------------
    protected:
        /** This is really a GLFWwindow* */
        void *glfwWindow;
    };
    
}}}



namespace trillek { namespace core {
#ifdef __doxygen__
    /** Typedef'd to the correct specialization of Window for the current
     *  platform. */
    typedef void PlatformWindow;
#endif
}}
