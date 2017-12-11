#pragma once

#include <string>
#include <vector>

namespace FileSpreader
{
    struct CommandLine
    {
        CommandLine(std::string const& cmdLine);

        std::vector <std::string> commands;
    };

    struct ProgramOptionVars
    {
        bool helpCalled = false;
        bool start = false;
        std::string port = "10102";
        std::string persistence = "";
        unsigned int refreshIntervalMs = 10000;
        unsigned int scanMax = 8192;
    };

    class ProgramOptions
    {
    public:
        ProgramOptions(CommandLine cmd);

        ProgramOptionVars getOptions() const;

    private:
        CommandLine cmd_;
        ProgramOptionVars vars_;
    };
}


