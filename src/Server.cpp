#include "Server.hpp"

extern bool isRunning;

Server::Server(int port, std::string password)
    : _port(port), _password(password) {
  std::time_t now = std::time(NULL);
  _creationTime = std::ctime(&now);
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
  std::cout << "Server is RUNNING..." << std::endl;
  while (isRunning) {
    // 1. Wait for events
    // &fds[0]  : Pointer to the first element of your vector
    // fds.size : Number of items to watch
    // -1       : Timeout (Wait forever until an event occurs) 
    int poll_count = poll(&_fds[0], _fds.size(), -1);

    if (poll_count < 0) {
      throw std::runtime_error("poll() failed");
    }

    // 2. Iterate through all file descriptors to check for events
    // We use a manual index because _fds.size() might grow inside the loop!
    for (size_t i = 0; i < _fds.size(); i++) {

      // Check if this specific socket has data to read (POLLIN)
      if (_fds[i].revents & POLLIN) {

        // CASE A: It's the Listener Socket (New Connection)
        if (_fds[i].fd == _serverSocketFd) {
          acceptNewClient();
        }
        // CASE B: It's a regular Client (Incoming Message)
        else {
          char buffer[1024];
          std::memset(buffer, 0, sizeof(buffer)); // Clear junk data

          ssize_t bytes = recv(_fds[i].fd, buffer, sizeof(buffer) - 1,
                               0); // -1 to leave space for null terminator

          if (bytes <= 0) {
            // Client disconnected or error
            std::cout << "Client FD " << _fds[i].fd << " disconnected."
                      << std::endl;
			disconnectClient(_fds[i].fd); //Ira: I put all stuff for client disconnection and poll fds deleting into his function
            i--; // Adjust index since vector shrank
          } else {
            // Data received!
            buffer[bytes] = '\0'; // Null terminate
            _clients[_fds[i].fd]->appendBuffer(buffer);

            std::cout << "Buffer for FD " << _fds[i].fd << ": "
                      << _clients[_fds[i].fd]->getBuffer() << std::endl;
			//Ira: extracting and executing commands
			while (_clients[_fds[i].fd] && _clients[_fds[i].fd]->getBuffer() != "") {
				std::string args = _clients[_fds[i].fd]->extractMessage();
				std::string cmd = extractCMD(args); 
				if (!executeCMD(cmd, args, *_clients[_fds[i].fd])) {
					_clients[_fds[i].fd]->clearBuffer();
					disconnectClient(_fds[i].fd);
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

void Server::acceptNewClient() {
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);

  // 1. Accept the connection
  int newFd =
      accept(_serverSocketFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
  if (newFd == -1) {
    std::cerr << "Error: accept failed" << std::endl;
    return;
  }

  // 2. Set Non-Blocking (Vital!)
  if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1) {
    std::cerr << "Error: fcntl failed" << std::endl;
    close(newFd);
    return;
  }

  // 3. Add to poll list
  struct pollfd newPfd;
  newPfd.fd = newFd;
  newPfd.events = POLLIN; // Monitor for incoming data
  newPfd.revents = 0;
  _fds.push_back(newPfd);

  Client *newClient = new Client(newFd, inet_ntoa(clientAddr.sin_addr));
  _clients[newFd] = newClient; // Store it in the map

  std::cout << "New Client Connected! FD: " << newFd << std::endl;
}

//Ira: close fd, deleting fd from poll fds, delete client obj and clean the clients map
void Server::disconnectClient(int fd) {
    std::cout << "Client FD " << fd << " disconnected." << std::endl;

    close(fd);
	deleteClientFromChannels(fd);
    delete _clients[fd];
    _clients.erase(fd);

    for (size_t i = 0; i < _fds.size(); ++i) {
        if (_fds[i].fd == fd) {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }
}

//Ira: if needed, can be deleted
void Server::sendPing(const Client& client) {
	std::string token = "keepalive";
    std::string msg = ":server PING :" + token + "\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
}

