#ifndef LOGGING_HPP_INCLUDED
#define LOGGING_HPP_INCLUDED

#include "trillek.hpp"
#include "type-id.hpp"
#include <memory>
#include <sstream>
#include <mutex>

#define LOGMSG(ll) trillek::Logging::Log<trillek::L_##ll, trillek::Logging>()
#define LOGMSGFOR(ll,x) trillek::Logging::Log<trillek::L_##ll, x>()
#define LOGMSGC(ll) trillek::Logging::Log<trillek::L_##ll>(*this)
#define LOGMSGON(ll,x) trillek::Logging::Log<trillek::L_##ll>(x)

namespace trillek {
enum LogLevel : int {
    L_FATAL,
    L_ERROR,
    L_WARNING,
    L_NOTICE,
    L_INFO,
    L_DEBUG,
    L_DEBUG_FINE
};
template<LogLevel T> std::string LogLevelString() { return "LOG"; }
template<> inline std::string LogLevelString<L_FATAL>() { return "FATAL"; }
template<> inline std::string LogLevelString<L_ERROR>() { return "ERROR"; }
template<> inline std::string LogLevelString<L_WARNING>() { return "WARNING"; }
template<> inline std::string LogLevelString<L_NOTICE>() { return "NOTICE"; }
template<> inline std::string LogLevelString<L_INFO>() { return "INFO"; }
template<> inline std::string LogLevelString<L_DEBUG>() { return "DEBUG"; }
template<> inline std::string LogLevelString<L_DEBUG_FINE>() { return "FINE"; }

class Logging {
private:
    Logging();
    static std::unique_ptr<Logging> instance;
    std::mutex log_m;
    static Logging& GetInstance();
public:

    void WriteLine(std::string l, std::string message);
    void WriteLine(std::string l, std::string section, std::string message);

    template<LogLevel T>
    class LogLine {
    public:
        LogLine() : show(true), glob(true) { }
        LogLine(std::string system) : show(true), glob(false), systemname(system) { }
        LogLine(const LogLine&) = delete;
        LogLine& operator=(const LogLine&) = delete;
        LogLine& operator=(LogLine&&) = delete;
        LogLine(LogLine&& that) : show(true), message(that.message.str()) {
            that.show = false;
            glob = that.glob;
            systemname = std::move(that.systemname);
        }
        ~LogLine() {
            if(show) {
                if(glob) {
                    GetInstance().WriteLine(LogLevelString<T>(), message.str());
                }
                else {
                    GetInstance().WriteLine(LogLevelString<T>(), systemname, message.str());
                }
            }
        }
        template<typename K>
        LogLine& operator<<(K m) {
            message << m;
            return *this;
        }
    private:
        bool show, glob;
        std::string systemname;
        std::ostringstream message;
    };

    template<LogLevel L, typename SYSTEM>
    static LogLine<L> Log() {
        if(reflection::GetTypeID<SYSTEM>() == ~0) {
            return LogLine<L>();
        }
        else {
            return LogLine<L>(reflection::GetTypeName<SYSTEM>());
        }
    }
    template<LogLevel L, typename SYSTEM>
    static LogLine<L> Log(const SYSTEM&) {
        if(reflection::GetTypeID<SYSTEM>() == ~0) {
            return LogLine<L>();
        }
        else {
            return LogLine<L>(reflection::GetTypeName<SYSTEM>());
        }
    }
};

} // namespace trillek

#endif
