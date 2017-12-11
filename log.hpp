#pragma once

#include <string>
#include <boost/preprocessor/stringize.hpp>

namespace FileSpreader
{
    enum class LogSeverity
    {
        Debug,
        Info,
        Warning,
        Error,
        Severe
    };

    #define LOG_CODE_PLACE BOOST_PP_STRINGIZE(__FILE__), BOOST_PP_STRINGIZE(__LINE__), __PRETTY_FUNCTION__

    void initLog(std::string const& fileName, bool enabled, LogSeverity level);
    void setLogConsoleOutputEnabled(bool enabled);
    void setLogLevel(LogSeverity level);

    void Log(LogSeverity severity, std::string const& msg, char const* file, char const* line, char const* function);
    void Log(LogSeverity severity, std::string const& msg);
    void Log(std::string const& msg); // severity = Info
    void Log(std::string const& msg, char const* file, char const* line, char const* function); // severity = Info
}
