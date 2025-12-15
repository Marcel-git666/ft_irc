#include "Server.hpp"
#include <iostream>

Server::Server(int port, std::string password) : _port(port), _password(password) {
}

Server::~Server() {
}

void Server::run() {
    std::cout << "Server is running on port " << _port << " with password '" << _password << "'" << std::endl;
    std::cout << "Listening for connections..." << std::endl;

    // TODO: Add the while(true) poll loop here.
    // For now, we just exit so you can verify the build works.
}