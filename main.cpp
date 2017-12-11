#include "main.hpp"

#include "log.hpp"

#include <boost/lexical_cast.hpp>

//#####################################################################################################################
int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int nCmdShow)
{
    using namespace FileSpreader;
    using namespace Rest;
    using namespace std::string_literals;

    try
    {
        std::cout << "Initializing Log.\n";
        initLog("file_spread_log", true, LogSeverity::Info);
    }
    catch (std::exception const& exc)
    {
        std::cerr << exc.what() << "\n";
        return 1;
    }

    ProgramOptions optParser(CommandLine{lpCmdLine});
    auto options = optParser.getOptions();

    std::cout << "Arguments parsed.\n";
    Log("Arguments parsed.");

    // do not execute anything if --help was passed
    if (options.helpCalled)
        return 0;

    try
    {
        // convert port
        uint32_t port;
        try
        {
            port = boost::lexical_cast <uint32_t> (options.port);
        }
        catch(...)
        {
            std::cerr << "Port must be a number!" << "\n";
            Log(LogSeverity::Error, "Port must be a number!", LOG_CODE_PLACE);
            return 1;
        }

        // Controller & Server creation.
        Controller controller{static_cast <int> (options.scanMax)};
        Server server (&controller, port);

        controller.setUpdateInterval(std::chrono::milliseconds(options.refreshIntervalMs));

        // load stored task, if given.
        if (!options.persistence.empty())
            controller.loadTasksFromFile(options.persistence);

        // start copy controller, if requested in commandLine
        if (options.start)
        {
            controller.start();
            std::cout << "Controller started.\n";
            Log("Controller started.");
        }

        // start REST interface.
        server.start();
        std::cout << "Interface up on port " << port << "\n";
        Log("Interface up on port "s + std::to_string(port) + ".");

        /// REMOVE ME
        /// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //ClonerOptions opts;
        //opts.setUseArchiveBit(true);
        //controller.addTask("D:/FileSpreadTest/Input", {"D:/FileSpreadTest/Output"}, opts);
        //controller.start();
        /// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // Create the window and enter the message loop:
        MSG messages;
        CreateHiddenWindow(messages, hThisInstance);

        Log("Dispatching messages now.", LOG_CODE_PLACE);
        while (GetMessage (&messages, NULL, 0, 0))
        {
            TranslateMessage(&messages);
            DispatchMessage(&messages);
        }

        Log("Shutting down...", LOG_CODE_PLACE);
        server.stop();

        // The program return-value is 0 - The value that PostQuitMessage() gave.
        return messages.wParam;
    }
    catch (std::exception const& exc)
    {
        Log(LogSeverity::Severe, exc.what(), LOG_CODE_PLACE);

        ShowMessage(exc.what());
        return 1;
    }
}
//#####################################################################################################################
