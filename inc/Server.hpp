#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <vector>
#include <poll.h>

class Server {
    private:
        int         _port;
        std::string _password;
        
        // This vector will hold the File Descriptors for poll()
        std::vector<struct pollfd> _fds;

    public:
        Server(int port, std::string password);
        ~Server();

        void run(); // The main loop
};

#endif