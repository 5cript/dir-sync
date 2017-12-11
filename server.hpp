#pragma once

#include "server_fwd.hpp"
#include "controller.hpp"

#include "SimpleREST/restful.hpp"

#include <cstdint>
#include <string>

namespace FileSpreader
{
    class Server
    {
    public:
        Server(Controller* controller, uint32_t port);

        void start();
        void stop();

    private: // internal functions
        void initialize();

    private:
        Rest::InterfaceProvider api_;
        Controller* controller_;
    };
}
