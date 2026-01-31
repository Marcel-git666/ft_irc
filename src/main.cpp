#include "Server.hpp"
#include <csignal> // <--- REQUIRED for signal()
#include <cstdlib>
#include <iostream>

// 1. Global flag to control the loop
bool isRunning = true;

// 2. The function that runs when you press CTRL+C
void signalHandler(int signum) {
  (void)signum; // Silence unused warning
  std::cout << "\nSignal received! Stopping server..." << std::endl;
  isRunning = false; // This will break the loop in Server.cpp
}

bool isValidPort(std::string const &port) {
  for (size_t i = 0; i < port.length(); i++) {
    if (!isdigit(port[i]))
      return false;
  }
  int p = std::atoi(port.c_str());
  return (p > 0 && p <= 65535);
}

int main(int argc, char **argv) {
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

  // 3. Register the signal handler
  signal(SIGINT, signalHandler);  // Catch Ctrl+C
  signal(SIGQUIT, signalHandler); // Catch Ctrl+backslash

  try {
    int port = std::atoi(portStr.c_str());
    Server server(port, password);

    std::cout << "Server started on port " << port << "..." << std::endl;

    // 4. Pass the flag to run() so it knows when to stop
    // Note: You need to update Server::run() to accept this flag
    // OR make isRunning extern in Server.cpp (easier for now).
    server.run();
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
