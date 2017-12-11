#include "log.hpp"

#include <boost/filesystem.hpp>

#include <fstream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>

namespace FileSpreader
{
    unsigned int const maxLogs = 10;

    bool logOut;
    LogSeverity lvl;
    std::string file = "";
    std::mutex unscramble;
    std::once_flag createLogFlag;

//#####################################################################################################################
    std::string getTimeStamp(bool spaceless = false)
    {
        std::stringstream sstr;
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::string pattern = "%d-%m-%Y %H-%M-%S";
        if (spaceless)
            pattern = "%d-%m-%Y_%H-%M-%S";

        sstr << std::put_time(&tm, pattern.c_str());
        return sstr.str();
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string determineLogName(std::string const& baseName)
    {
        namespace fs = boost::filesystem;

        std::string logDirectory = "./logs/";

        if (fs::exists(logDirectory) && !fs::is_directory(logDirectory))
            throw std::runtime_error("There must not be a logs file in the program directory");

        if (!fs::exists(logDirectory))
            fs::create_directory(logDirectory);

        uint32_t cnt = 0;
        fs::path oldest = "";
        std::time_t oldestTime = std::time_t(nullptr);

        for (fs::directory_iterator i(logDirectory), end; i != end; ++i)
        {
            auto name = i->path().filename().string();
            if (fs::is_regular_file(i->status()) && name.length() > baseName.length() && name.substr(0, baseName.length()) == baseName)
            {
                ++cnt;
                auto modTime = fs::last_write_time(i->path());
                if (oldestTime == 0 || std::difftime(oldestTime, modTime) > 0)
                {
                    oldest = i->path();
                    oldestTime = modTime;
                }
            }
        }

        if (cnt >= maxLogs)
            fs::remove(oldest);

        return logDirectory + baseName + "_" + getTimeStamp(true) + ".log";
    }
//---------------------------------------------------------------------------------------------------------------------
    void initLog(std::string const& fileName, bool enabled, LogSeverity level)
    {
        file = determineLogName(fileName);
        logOut = enabled;
        lvl = level;
    }
//---------------------------------------------------------------------------------------------------------------------
    void setLogConsoleOutputEnabled(bool enabled)
    {
        logOut = enabled;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::string severityToString(LogSeverity severity)
    {
        switch (severity)
        {
        case LogSeverity::Debug:
            return "DEBUG";
        case LogSeverity::Info:
            return "INFO";
        case LogSeverity::Warning:
            return "WARNING";
        case LogSeverity::Error:
            return "ERROR";
        case LogSeverity::Severe:
            return "SEVERE";
        default:
            return "";
        }
    }
//---------------------------------------------------------------------------------------------------------------------
    void setLogLevel(LogSeverity level)
    {
        lvl = level;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::ofstream& getLogFile()
    {
        static std::ofstream stream(file, std::ios_base::binary);
        return stream;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Log(LogSeverity severity, std::string const& msg, char const* file, char const* line, char const* function)
    {
        if ((int)severity < (int)lvl)
            return;

        auto& stream = getLogFile();

        std::lock_guard <std::mutex> lock(unscramble);

        stream << "[" << severityToString(severity) << "] (" << getTimeStamp() << ") {" << file << ":" << line << " in \"" << function << "\"}: " << msg << std::endl;
        if (logOut)
            std::cout << "[" << severityToString(severity) << "] (" << getTimeStamp() << ") {" << file << ":" << line << " in \"" << function << "\"}: " << msg << std::endl;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Log(LogSeverity severity, std::string const& msg)
    {
        if ((int)severity < (int)lvl)
            return;

        auto& stream = getLogFile();

        std::lock_guard <std::mutex> lock(unscramble);

        stream << "[" << severityToString(severity) << "] (" << getTimeStamp() << "): " << msg << std::endl;
        if (logOut)
            std::cout << "[" << severityToString(severity) << "] (" << getTimeStamp() << "): " << msg << std::endl;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Log(std::string const& msg) // severity = Info
    {
        Log(LogSeverity::Info, msg);
    }
//---------------------------------------------------------------------------------------------------------------------
    void Log(std::string const& msg, char const* file, char const* line, char const* function)
    {
        Log(LogSeverity::Info, msg, file, line, function);
    }
//#####################################################################################################################
}
