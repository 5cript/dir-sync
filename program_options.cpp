#include "program_options.hpp"
#include "windows.hpp"

#include <boost/program_options.hpp>

#include <locale>
#include <codecvt>
#include <iostream>

namespace FileSpreader
{
//#####################################################################################################################
    template<class Facet>
    struct deletable_facet : Facet
    {
        template<class ...Args>
        deletable_facet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
        ~deletable_facet() {}
    };
//---------------------------------------------------------------------------------------------------------------------
    std::string wideToNarrow(const std::wstring &utf16)
    {
        std::wstring_convert<
            deletable_facet<
                std::codecvt<wchar_t, char, std::mbstate_t>
            >, wchar_t
        > converter;
        return converter.to_bytes(utf16);
    }
//---------------------------------------------------------------------------------------------------------------------
    std::wstring narrowToWide(const std::string &utf8)
    {
        std::wstring_convert<
            deletable_facet<
                std::codecvt<wchar_t, char, std::mbstate_t>
            >, wchar_t
        > converter;
        return converter.from_bytes(utf8);
    }
//#####################################################################################################################
    ProgramOptions::ProgramOptions(CommandLine cmd)
        : cmd_(std::move(cmd))
        , vars_()
    {
        namespace po = boost::program_options;

        po::options_description desc("Possible options");

        desc.add_options()
            ("help", "produce help message")
            ("port,p", po::value <std::string>(&vars_.port), "binding port for REST interface")
            ("start,s", po::bool_switch(&vars_.start), "starts copy operation immediately")
            ("tasks,t", po::value <std::string>(&vars_.persistence), "a persistence file, with saved tasks")
            ("interval,i", po::value <unsigned int>(&vars_.refreshIntervalMs), "The refresh interval in milliseconds, must be larger than 100ms.")
            ("scanFileMax,m", po::value <unsigned int>(&vars_.scanMax), "Maximum files to be scanned each round")
        ;

        std::vector <char const*> prox;
        for (auto const& i : cmd_.commands) {
            prox.push_back(i.data());
        }

        po::variables_map vm;
        po::store(po::parse_command_line(cmd_.commands.size(), prox.data(), desc), vm);
        po::notify(vm);

        // print help and set help flag.
        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            vars_.helpCalled = true;
            return;
        }

        if (vars_.refreshIntervalMs < 100)
            vars_.refreshIntervalMs = 1000;
    }
//---------------------------------------------------------------------------------------------------------------------
    ProgramOptionVars ProgramOptions::getOptions() const
    {
        return vars_;
    }
//#####################################################################################################################
    CommandLine::CommandLine(std::string const& cmdLine)
    {
        char myPath[_MAX_PATH+1];
        GetModuleFileName(nullptr, myPath, _MAX_PATH);

        commands.push_back(myPath);

        auto wCmdLine = narrowToWide(cmdLine);

        int argc = 0;
        LPWSTR* argList = CommandLineToArgvW(wCmdLine.c_str(), &argc);

        if (argList == nullptr)
            return;

        for (int i = 0; i != argc; ++i)
        {
            std::string str  = wideToNarrow({argList[i]});
            commands.push_back(str);
        }

        LocalFree(argList);
    }
//#####################################################################################################################
}
