#include "../inc/Server.hpp"

extern volatile sig_atomic_t g_signaled;

Server::Server(int port, const std::string &password)
    : _is_running(true), _port(port), _password(password) {
  std::time_t now = std::time(NULL);
  _creationTime = std::ctime(&now);
  init();
}

Server::~Server() { cleanMemory(); }

/**
 * @brief Initializes the server socket, sets options, and binds to the port.
 * Sets the socket to non-blocking mode and prepares the poll structure.
 */
void Server::init() {
  _serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (_serverSocketFd == -1) {
    throw std::runtime_error("Error: Failed to create socket");
  }

  std::cout << "Server initialized! Socket FD: " << _serverSocketFd
            << std::endl;

  int option_value = 1;
  // Allow reuse of local addresses to prevent "Address already in use" errors
  // after restart
  if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &option_value,
                 sizeof(option_value)) == -1) {
    throw std::runtime_error("Error: failed to set socket options");
  }

  // Set Non-Blocking Mode
  if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1) {
    throw std::runtime_error("Error: failed to set non-blocking mode");
  }

  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
  addr.sin_port = htons(_port);      // Convert to Network Byte Order

  if (bind(_serverSocketFd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    throw std::runtime_error("Error: failed to bind socket");
  }

  if (listen(_serverSocketFd, SOMAXCONN) == -1) {
    throw std::runtime_error("Error: failed to listen");
  }

  std::cout << "Server is listening on port " << _port << "..." << std::endl;

  // Add the listener socket to our poll system
  struct pollfd pfd;
  pfd.fd = _serverSocketFd;
  pfd.events = POLLIN;
  pfd.revents = 0;
  _fds.push_back(pfd);
}

/**
 * @brief The main event loop of the server.
 * Waits for incoming data or new connections using poll() and processes them.
 */
void Server::run() {
  std::cout << "Server is RUNNING..." << std::endl;

  while (_is_running) {
    int poll_count = poll(&_fds[0], _fds.size(), -1);

    // Check if Ctrl+C was pressed
    if (g_signaled) {
      std::cout << "\nShutting down server..." << std::endl;
      _is_running = false;
      break;
    }

    if (poll_count < 0) {
      throw std::runtime_error("poll() failed");
    }

    for (size_t i = 0; i < _fds.size(); i++) {
      if (_fds[i].revents & POLLIN) {

        // CASE A: New incoming connection
        if (_fds[i].fd == _serverSocketFd) {
          acceptNewClient();
        }
        // CASE B: Data received from an existing client
        else {
          char buffer[1024];
          std::memset(buffer, 0, sizeof(buffer));

          ssize_t bytes = recv(_fds[i].fd, buffer, sizeof(buffer) - 1, 0);

          if (bytes <= 0) {
            std::cout << "Client FD " << _fds[i].fd << " disconnected."
                      << std::endl;
            disconnectClient(_fds[i].fd, "");
            i--; // Adjust index since the vector shrank
          } else {
            // Data received safely
            _clients[_fds[i].fd]->appendBuffer(buffer);
            int current_fd = _fds[i].fd;

            // FIX: Process messages ONLY when a full line (\n) is ready in the
            // buffer
            while (_clients.find(current_fd) != _clients.end() &&
                   _clients[current_fd]->isReady()) {
              std::string args = _clients[current_fd]->extractMessage();

              if (!executeCMD(args, *_clients[current_fd])) {
                if (_clients.find(current_fd) != _clients.end()) {
                  _clients[current_fd]->clearBuffer();
                  disconnectClient(current_fd, "");
                }
                i--;
                break;
              }
              // If client was disconnected inside executeCMD (e.g., by QUIT
              // command)
              if (_clients.find(current_fd) == _clients.end()) {
                i--;
                break;
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @brief Accepts a new client connection, sets it to non-blocking, and adds it
 * to the server.
 */
void Server::acceptNewClient() {
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);

  int newFd =
      accept(_serverSocketFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
  if (newFd == -1) {
    std::cerr << "Error: accept failed" << std::endl;
    return;
  }

  // Set the new client socket to non-blocking
  if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1) {
    std::cerr << "Error: fcntl failed" << std::endl;
    close(newFd);
    return;
  }

  struct pollfd newPfd;
  newPfd.fd = newFd;
  newPfd.events = POLLIN;
  newPfd.revents = 0;
  _fds.push_back(newPfd);

  Client *newClient = new Client(newFd, inet_ntoa(clientAddr.sin_addr));
  _clients[newFd] = newClient;

  std::cout << "New Client Connected! FD: " << newFd << std::endl;
}

/**
 * @brief Fully disconnects a client, cleans up their FD, and removes them from
 * channels.
 */
void Server::disconnectClient(int fd, std::string args) {
  deleteClientFromChannels(fd, args);
  close(fd);
  delete _clients[fd];
  _clients.erase(fd);

  for (size_t i = 0; i < _fds.size(); ++i) {
    if (_fds[i].fd == fd) {
      _fds.erase(_fds.begin() + i);
      break;
    }
  }
  std::cout << "Client FD " << fd << " disconnected." << std::endl;
}

/**
 * @brief Helper for unit testing to add a mocked client.
 */
void Server::addClient(Client &client) { _clients[client.getFd()] = &client; }

/**
 * @brief Frees all allocated memory and closes all open file descriptors upon
 * server shutdown.
 */
void Server::cleanMemory() {
  if (!_channels.empty()) {
    _channels.clear();
  }
  if (!_clients.empty()) {
    for (std::map<int, Client *>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
      close(it->first);
      delete it->second;
    }
    _clients.clear();
  }
  close(_serverSocketFd);
}
