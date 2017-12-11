#include "server.hpp"

#include "all_messages.hpp"
#include "log.hpp"
#include "version.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "main_window.hpp"

namespace FileSpreader
{
//#####################################################################################################################
    Server::Server(Controller* controller, uint32_t port)
        : api_(port)
        , controller_(controller)
    {
        initialize();
    }
//---------------------------------------------------------------------------------------------------------------------
    void Server::start()
    {
        api_.start();
    }
//---------------------------------------------------------------------------------------------------------------------
    void Server::stop()
    {
        api_.stop();
    }
//---------------------------------------------------------------------------------------------------------------------
    void Server::initialize()
    {
        using namespace Rest;
        using namespace std::string_literals;

        api_.get("/test/:echo", [](Request request, Response response)
        {
            response.status(200).send(request.getParameter("echo") + "\r\n");
        });

        /*
        api_.get("/spam/me", [](Request request, Response response)
        {
            std::string big;
            for (std::size_t i = 0; i != 1024*1024; ++i)
                big.push_back(i%26 + 'A');

            response.status(200).send(big);
        });
        */

        api_.post("/interval/:ms", [this](Request request, Response response)
        {
            auto timeStr = request.getParameter("ms");
            auto timeMs = std::atol(timeStr.c_str());
            if (timeMs == 0 && !timeStr.empty() && timeStr[0] != '0')
            {
                Log(LogSeverity::Warning, "/interval/:ms was called with invalid millisecond parameter.");
                response.status(400).send("Expected a time in milliseconds, but got "s + timeStr);
                return;
            }

            controller_->setUpdateInterval(std::chrono::milliseconds{timeMs});
            response.sendStatus(204);
        });

        // Putting a task is idempotent, as the sources are maps and it is not possible
        // to have the same source directory multiple times.
        // Putting a source directory with different destination directories will overwrite the task
        // associated with the source and all current operations will be stopped.
        api_.put("/task", [this](Request request, Response response)
        {
            try
            {
                auto task = request.getJson <Messages::Task> ();

                // no error with the json.
                std::vector <std::string> destinations = task.getDestinations();
                ClonerOptions options = ClonerOptionsFromMessage(task);

                controller_->addTask(task.source, destinations, options);

                // ok dokey
                response.sendStatus(204);
            }
            catch (boost::property_tree::ptree_bad_data const& exc)
            {
                response.status(400).send("bad data");
                Log(LogSeverity::Warning, exc.what(), LOG_CODE_PLACE);
                return;
            }
            catch (boost::property_tree::ptree_bad_path const& exc)
            {
                response.status(400).send("json does not contain needed keys");
                Log(LogSeverity::Warning, exc.what(), LOG_CODE_PLACE);
                return;
            }
            catch(std::exception const& exc)
            {
                response.status(500).send(exc.what());
                Log(LogSeverity::Severe, exc.what(), LOG_CODE_PLACE);
                return;
            }
        });

        // starts the file spreading
        api_.post("/start", [this](Request request, Response response)
        {
            try
            {
                controller_->start();
                response.sendStatus(204);
            }
            catch (std::exception const& exc)
            {
                response.status(500).send(exc.what());
                Log(LogSeverity::Severe, exc.what(), LOG_CODE_PLACE);
            }
        });

        // pauses the file spreading
        api_.post("/pause", [this](Request request, Response response)
        {
            try
            {
                controller_->pause();
                response.sendStatus(204);
            }
            catch (std::exception const& exc)
            {
                response.status(500).send(exc.what());
                Log(LogSeverity::Severe, exc.what(), LOG_CODE_PLACE);
            }
        });

        // gets the last spreader error, if there was one.
        api_.get("/error", [this](Request request, Response response)
        {
            response.status(200).send(controller_->getLastError() + "\r\n");
        });

        // gets the running state.
        api_.get("/running", [this](Request request, Response response)
        {
            std::stringstream sstr;
            sstr << std::boolalpha << controller_->isRunning();
            response.status(200).send(sstr.str());
        });

        api_.get("/progress", [this](Request request, Response response)
        {
            auto report = controller_->compileProgressReport(request.getQuery()["verbose"]=="true", request.getQuery()["stotal"]=="true");
            response.json <ProgressReport> (report);
        });

        api_.get("/progress/xml", [this](Request request, Response response)
        {
            auto report = controller_->compileProgressReport(request.getQuery()["verbose"]=="true", request.getQuery()["stotal"]=="true");
            response.xml <ProgressReport> (report);
        });

        api_.post("/save", [this](Request request, Response response)
        {
            try
            {
                auto file = request.getJson <Messages::File> ();

                if (!controller_->saveTasksToFile(file.fileName))
                {
                    response.status(400).send("invalid file name");
                    Log(LogSeverity::Warning, "Invalid file name.", LOG_CODE_PLACE);
                }
                else
                    response.status(204).end();
            }
            catch (boost::property_tree::ptree_bad_data const& exc)
            {
                response.status(400).send("bad data");
                Log(LogSeverity::Error, exc.what(), LOG_CODE_PLACE);
                return;
            }
            catch (boost::property_tree::ptree_bad_path const& exc)
            {
                response.status(400).send("json does not contain needed keys");
                Log(LogSeverity::Error, exc.what(), LOG_CODE_PLACE);
                return;
            }
            catch(std::exception const& exc)
            {
                response.status(500).send(exc.what());
                Log(LogSeverity::Severe, exc.what(), LOG_CODE_PLACE);
                return;
            }
        });

        api_.post("/load", [this](Request request, Response response)
        {
            if (controller_->isRunning())
            {
                response.status(400).send("cannot load while running.");
                return;
            }

            try
            {
                auto file = request.getJson <Messages::File> ();

                try
                {
                    if (!controller_->loadTasksFromFile(file.fileName))
                    {
                        response.status(400).send("invalid file name");
                        Log(LogSeverity::Warning, "Invalid file name.", LOG_CODE_PLACE);
                    }
                    else
                        response.status(204).end();
                }
                catch(std::exception const& exc)
                {
                    response.status(400).send(exc.what());
                    Log(LogSeverity::Error, exc.what(), LOG_CODE_PLACE);
                }
            }
            catch (boost::property_tree::ptree_bad_data const& exc)
            {
                response.status(400).send("bad data");
                Log(LogSeverity::Error, exc.what(), LOG_CODE_PLACE);
                return;
            }
            catch (boost::property_tree::ptree_bad_path const& exc)
            {
                response.status(400).send("json does not contain needed keys");
                Log(LogSeverity::Error, exc.what(), LOG_CODE_PLACE);
                return;
            }
            catch(std::exception const& exc)
            {
                response.status(500).send(exc.what());
                Log(LogSeverity::Severe, exc.what(), LOG_CODE_PLACE);
                return;
            }
        });

        api_.get("/window", [this](Request request, Response response)
        {
            response.status(200).send(std::to_string(reinterpret_cast <uint64_t> (window)));
        });

        api_.get("/version", [this](Request request, Response response)
        {
            response.status(200).send(AutoVersion::FULLVERSION_STRING);
        });
    }
//#####################################################################################################################
}
