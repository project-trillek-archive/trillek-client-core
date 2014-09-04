//=============================================================================
//! \ingroup CORE
//! \file   AssertionMacros.h
//! \brief  Macros for assertions, optionally under specific conditions.
//
//! \ingroup CORE
//! \defgroup ASSERTIONS Assertions
//! \brief  Macros for assertions, optionally under specific conditions.
//
//! @namespace trillek::core::assertion
//! \ingroup ASSERTIONS
//! \brief Namespace for the \ref ASSERTIONS group of the Core module.
//
//! \addtogroup ASSERTIONS
//! \{
//=============================================================================

namespace trillek { namespace core { namespace assertion {
    
    /** You should not access this class directly.  Use the macros in 
     *  \ref ASSERTIONS instead. */
    struct Handler {
        /** Handler for assertion failures.  Throws a \c std::runtime_error. */
        static void AssertionFailed(const char *expression, const char *file, int line, const char *message="", ...);
    };
    
}}};


//-----------------------------------------------------------------------------
#pragma mark -  Checks
//! \name       Checks
//!
//! \brief      Checks are pedantic assertions.  They are preprocessed away
//!             if \c ENABLE_CHECK is disabled and therefore incur no
//!             performance penalty in a shipping build.  Conversely,
//!             you should not use a \c check for any expression that
//!             must be executed, but not necessarily, evaluated all the time.
//-----------------------------------------------------------------------------

/** Invokes the assertion handler if a given condition evaluates to false. 
 *
 *  \note
 *  If \c ENABLE_CHECK is not enabled, this statement is pre-processed away
 *  and the expression will not execute. 
 */
#ifndef __doxygen__
    #define check(expr) checkWithAction(expr, assertion::Handler::AssertionFailed( metamacro_stringify(expr), __FILE__, __LINE__ ))
#else
    #define check(expr)
#endif


/** Invokes the assertion handler if a given condition evaluates to false.
 *
 *  Analogous to \ref check but with the option of printing a custom message
 *  should the assertion fail.
 *
 *  \note
 *  If \c ENABLE_CHECK is not enabled, this statement is pre-processed away
 *  and the expression will not execute.
 */
#ifndef __doxygen__
    #define checkf(expr, ...) checkWithAction(expr, assertion::Handler::AssertionFailed( metamacro_stringify(expr), __FILE__, __LINE__, ##__VA_ARGS__ ))
#else
    #define checkf(expr, ...)
#endif


/** Executes \a code if the given expression evaluates to \c false.
 *
 *  Use of this macro is analogous to the following code:
 * 
 *      if (!expr) {
 *          // Handle false condition
 *      }
 *
 *  \note
 *  If \c ENABLE_CHECK is not enabled, this statement is pre-processed away
 *  and the expression will not execute. 
 */
#if ENABLE_CHECK
    #define checkWithAction(expr, code) \
        { do { if(!(expr)) { code; } } while (0); }
#else
    #define checkWithAction(expr, code)
#endif


//-----------------------------------------------------------------------------
#pragma mark -  Verification
//! \name       Verification
//!
//! \brief      The \c verify family of assertions are a hybrid of \c check
//!             and \c require.  The provided expression is always executed
//!             but the result is only evaluated if \c ENABLE_CHECK is enabled.
//-----------------------------------------------------------------------------

/** Invokes the assertion handler if a given condition evaluates to false.
 *
 *  \note
 *  If \c ENABLE_CHECK is not enabled, the expression is still executed
 *  but it's result is not evaluated.
 */
#ifndef __doxygen__
    #define verify(expr) verifyWithAction(expr, assertion::Handler::AssertionFailed( metamacro_stringify(expr), __FILE__, __LINE__ ))
#else
    #define verify(expr)
#endif


/** Invokes the assertion handler if a given condition evaluates to false.
 *
 *  Analogous to \ref verify but with the option of printing a custom message
 *  should the assertion fail.
 *
 *  \note
 *  If \c ENABLE_CHECK is not enabled, the expression is still executed
 *  but it's result is not evaluated.
 */
#ifndef __doxygen__
    #define verifyf(expr, ...) verifykWithAction(expr, assertion::Handler::AssertionFailed( metamacro_stringify(expr), __FILE__, __LINE__, ##__VA_ARGS__ ))
#else
    #define verifyf(expr, ...)
#endif


/** Executes \a code if the given expression evaluates to \c false.
 *
 *  Use of this macro is analogous to the following code:
 *
 *      if (!expr) {
 *          // Handle false condition
 *      }
 *
 *  \note
 *  If \c ENABLE_CHECK is not enabled, the expression is still executed
 *  but it's result is not evaluated.
 */
#if ENABLE_CHECK
    #define verifyWithAction(expr, code) \
        { do { if(!(expr)) { code; } } while (0); }
#else
    #define verifyWithAction(expr, code) { expr; }
#endif


//-----------------------------------------------------------------------------
#pragma mark -  Requirements
//! \name       Requirements
//!
//! \brief      Requirements are assertions that can not be disabled.  Use
//!             them sparingly as they will always carry a performance cost
//!             as well as a user experience cost when the program comes
//!             crashing down because an assertion failed.
//-----------------------------------------------------------------------------

/** Invokes the assertion handler if a given condition evaluates to false. */
#ifndef __doxygen__
    #define require(expr) requireWithAction(expr, assertion::Handler::AssertionFailed( metamacro_stringify(expr), __FILE__, __LINE__ ))
#else
    #define require(expr)
#endif


/** Invokes the assertion handler if a given condition evaluates to false.
 *
 *  Analogous to \ref require but with the option of printing a custom message
 *  should the assertion fail.
 */
#ifndef __doxygen__
    #define requiref(expr, ...) requireWithAction(expr, assertion::Handler::AssertionFailed( metamacro_stringify(expr), __FILE__, __LINE__, ##__VA_ARGS__ ))
#else
    #define requiref(expr, ...)
#endif


/** Executes \a code if the given expression evaluates to \c false.
 *
 *  Use of this macro is analogous to the following code:
 *
 *      if (!expr) {
 *          // Handle false condition
 *      }
 */
#ifndef __doxygen__
    #define requireWithAction(expr, code) \
        { do { if(!(expr)) { code; } } while (0); }
#else
    #define requireWithAction(expr, code)
#endif


//=============================================================================
//! \}
