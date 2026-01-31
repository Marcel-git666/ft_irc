#include "Server.hpp"
#include <cstring> // <--- REQUIRED for std::memset
#include <fcntl.h> // <--- REQUIRED for fcntl, F_SETFL, O_NONBLOCK
#include <iostream>
#include <netinet/in.h> // Required for sockaddr_in
#include <stdexcept>
#include <sys/socket.h>

Server::Server(int port, std::string password)
    : _port(port), _password(password) {
  init();
}

Server::~Server() {}

void Server::init() {
  // A. Create the Socket
  // AF_INET = IPv4, SOCK_STREAM = TCP
  _serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);

  if (_serverSocketFd == -1) {
    throw std::runtime_error("Error: Failed to create socket");
  }

  std::cout << "Server initialized! Socket FD: " << _serverSocketFd
            << std::endl;
  int option_value = 1; // 1 means "True" -> Turn this option ON

  // 2. Apply the option
  // Level: SOL_SOCKET (Socket-level option)
  // Option: SO_REUSEADDR (Allow reuse of local addresses)
  if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &option_value,
                 sizeof(option_value)) == -1) {
    throw std::runtime_error("Error: failed to set socket options");
  }

  std::cout << "Socket options set to reuse address." << std::endl;

  // 3. Set Non-Blocking Mode
  // F_GETFL: Get current flags
  // F_SETFL: Set new flags
  // O_NONBLOCK: The specific flag for non-blocking I/O
  if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1) {
    throw std::runtime_error("Error: failed to set non-blocking mode");
  }

  std::cout << "Socket set to non-blocking mode." << std::endl;

  // 4. Create the address structure
  struct sockaddr_in addr;
  // Clear the structure (good practice to avoid garbage values)
  std::memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET; // IPv4
  // INADDR_ANY tells the OS: "Listen on ALL available network interfaces"
  // (e.g., Wifi, Ethernet, and Localhost)
  addr.sin_addr.s_addr = INADDR_ANY;
  // htons converts "Host Byte Order" to "Network Byte Order" (Big Endian)
  // If you forget this, port 6667 might look like port 42351 to the network!
  addr.sin_port = htons(_port);

  // 5. Bind the socket to the port
  if (bind(_serverSocketFd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    throw std::runtime_error("Error: failed to bind socket");
  }
  std::cout << "Socket bound to port " << _port << "." << std::endl;

  // 6. Listen for incoming connections
  // SOMAXCONN is a constant (usually 128 or 4096) that defines the maximum
  // number of pending connections in the queue.
  // If 1000 people try to connect at the EXACT same nanosecond, the OS queues
  // them.
  if (listen(_serverSocketFd, SOMAXCONN) == -1) {
    throw std::runtime_error("Error: failed to listen");
  }

  std::cout << "Server is listening on port " << _port << "..." << std::endl;

  // 7. Add the server socket to the poll structure
  struct pollfd pfd;
  pfd.fd = _serverSocketFd; // Watch this file descriptor
  pfd.events = POLLIN;      // We care about "POLLIN" (Data coming IN)
  pfd.revents = 0;          // Clear the return events

  _fds.push_back(pfd); // Add it to your vector
}

void Server::run() {
  std::cout << "Server is running on port " << _port << " with password '"
            << _password << "'" << std::endl;
  std::cout << "Listening for connections..." << std::endl;

  // TODO: Add the while(true) poll loop here.
  // For now, we just exit so you can verify the build works.
}
