#include <iostream>
#include <cstdlib> // For atoi, exit
#include "Server.hpp" // We will create this next

bool isValidPort(std::string const &port) {
    // Check if empty or contains non-digits
    for (size_t i = 0; i < port.length(); i++) {
        if (!isdigit(port[i]))
            return false;
    }
    // Check range (0-65535), though strictly ports < 1024 require sudo
    int p = std::atoi(port.c_str());
    return (p > 0 && p <= 65535);
}

int main(int argc, char **argv) {
    // 1. Validate Arguments
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    std::string portStr = argv[1];
    std::string password = argv[2];

    if (!isValidPort(portStr)) {
        std::cerr << "Error: Invalid port number." << std::endl;
        return 1;
    }

    // 2. Start Server
    try {
        int port = std::atoi(portStr.c_str());
        
        // This is where your Server class takes over
        Server server(port, password);
        
        std::cout << "Server started on port " << port << "..." << std::endl;
        server.run(); // The infinite poll loop
    }
    catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}