//=============================================================================
//! \ingroup CORE
//! \file   ClassBase.h
//=============================================================================

//-------------------------------------------------------------------------
#pragma mark    Declaring Classes
//! \name       Declaring Classes
//-------------------------------------------------------------------------

//! \internal
#define DECLARE_CLASS_1(TClass) \
private: \
    typedef TClass ThisClass; \
    typedef std::unique_ptr<ThisClass> ThisPtr; \
    typedef std::unique_ptr<ThisClass>& ThisRef;

//! \internal
#define DECLARE_CLASS_2(TClass, TSuperClass) \
private: \
    typedef TClass ThisClass; \
    typedef TClass Super; \
    typedef std::unique_ptr<ThisClass> ThisPtr; \
    typedef std::unique_ptr<ThisClass>& ThisRef;

/** Declares some types and methods common to all classes. */
#define DECLARE_CLASS(...) \
    metamacro_concat(DECLARE_CLASS_, metamacro_argcount(__VA_ARGS__))(__VA_ARGS__) \
    protected:


#define CORE_API(TClass) \
    typedef std::unique_ptr<class TClass>  metamacro_concat(TClass, Ptr); \
    typedef std::unique_ptr<class TClass>& metamacro_concat(TClass, Ref); \
    typedef std::shared_ptr<class TClass>  metamacro_concat(TClass, SharedPtr); \
    typedef std::shared_ptr<class TClass>& metamacro_concat(TClass, SharedRef); \
    typedef std::weak_ptr<class TClass>  metamacro_concat(TClass, WeakPtr); \
    typedef std::weak_ptr<class TClass>& metamacro_concat(TClass, WeakRef);


#define REEXPORT_API(TType, TName) \
    typedef metamacro_concat(TType, Ptr) metamacro_concat(TName, Ptr); \
    typedef metamacro_concat(TType, Ref) metamacro_concat(TName, Ref); \
    typedef metamacro_concat(TType, SharedPtr) metamacro_concat(TName, SharedPtr); \
    typedef metamacro_concat(TType, SharedRef) metamacro_concat(TName, SharedRef); \
    typedef metamacro_concat(TType, WeakPtr) metamacro_concat(TName, WeakPtr); \
    typedef metamacro_concat(TType, WeakRef) metamacro_concat(TName, WeakRef);

