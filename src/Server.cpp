#include "Server.hpp"
#include <arpa/inet.h> // for inet_ntoa
#include <cstring>     // <--- REQUIRED for std::memset
#include <fcntl.h>     // <--- REQUIRED for fcntl, F_SETFL, O_NONBLOCK
#include <iostream>
#include <netinet/in.h> // Required for sockaddr_in
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h> // for close()
#include <cctype> // Ira: for isdigit nickname checking
#include <ctime> // Ira: for PING client, keeping him alive

extern bool isRunning;

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
			while (_clients[_fds[i].fd]->getBuffer() != "") {
				std::string args = _clients[_fds[i].fd]->extractMessage();
				std::string cmd = extractCMD(args); 
				if (!executeCMD(cmd, args, _clients[_fds[i].fd])) {
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
    delete _clients[fd];
    _clients.erase(fd);

    for (size_t i = 0; i < _fds.size(); ++i) {
        if (_fds[i].fd == fd) {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }
}

//Ira: extract command, and cut the message to the args
std::string Server::extractCMD(std::string& args) {
	size_t pos = args.find(' ');
	std::string command = args.substr(0, pos);
	std::cout << "Command from client: " << command;
	args = args.substr(pos + 1, args.length());
	std::cout << " ARGS for this command: " << args << "." << std::endl;
	return command;
}

//Ira: check Nickname on Uniqueness and rules
std::string Server::checkNickname(std::string arg) {
	if (!_clients.empty()) {
		for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
			if (it->second->getNickname() == arg)
				return ("433");
		}
	}
	if (isdigit(arg[0]) || arg.find(':') != std::string::npos
		|| arg.find('#') != std::string::npos || arg.find(' ') != std::string::npos)
			return("432");
	return ("ok");
}

//Ira: if needed, can be deleted
void Server::sendPing(Client* client) {
	std::string token = "keepalive";
    std::string msg = ":server PING :" + token + "\r\n";
    send(client->getFd(), msg.c_str(), msg.size(), 0);
}

//Ira: execute commands
bool Server::executeCMD(std::string cmd, std::string args, Client* client) {
	if (cmd == "PASS") {
		if (args != _password) {
			if (DEBUG)
				std::cout << RED << "Password doesn't match" << ENDCOLOR << std::endl;
			sendError(args, 464, client);
			return (false); //Ira: returned here because if password is wrong we don't accept anything else and immediately disconnect this client
		}
		client->setHasPassword();
		if (DEBUG)
			std::cout << GREEN << "Client FD " << client->getFd() << " has input correct password" << ENDCOLOR << std::endl;
	}
	else if (cmd == "NICK") {
		std::string checkNick = checkNickname(args);
		if (checkNick != "ok") {
			if (checkNick == "433")
				sendError(args, 433, client);
			else if (checkNick == "432")
				sendError(args, 432, client);
			return (true);
		}
		else {
			if (client->getNickname() != "") {
				// [TO DO] (if NICK changes, need to send info for other users)
				if (DEBUG)
					std::cout << GREEN << "Nickname of client FD " << client->getFd() << " changed to: " << args << ENDCOLOR << std::endl;
			}
			else {
				if (DEBUG)
					std::cout << "Nickname of new client FD " << client->getFd() << ": " << args << std::endl;
			}
			client->setNickname(args);
		}
	}
	else if (cmd == "USER") {
		if (!client->getRegistered()) {
			//Ira: USER command looks like USER <username> 0 * :<realname>
			//clients usually use for username and realname the nickname, if it's not set by user
			//to expect an empty username is an extra safety, but we have error massage for it so I included it
			std::string username = args.substr(0, args.find(' ')); //find first arg
			if (username.empty())
				sendError(args, 461, client);
			if (username.length() > USERLEN) {
				username = username.substr(0, USERLEN);
				//Ira: IRCstandard: "If this length is advertised, the username 
				//MUST be silently truncated to the given length before being used." 
			}
			if (username[0] != '~')
				username.insert(0, 1, '~');
				//Ira: IRCstandard: "... the username provided by the client SHOULD be prefixed 
				// by a tilde ('~', 0x7E) to show that this value is user-set."
			client->setUsername(username);
			std::cout << GREEN << "Client FD " << client->getFd() << " username is " << client->getUsername() << ENDCOLOR << std::endl;
			std::string realname = args.substr(args.find(':') + 1);
			client->setRealname(realname);
			std::cout << GREEN << "Client FD " << client->getFd() << " realname is " << client->getRealname() << ENDCOLOR << std::endl;
			registerClient(*client); //Ira: registration if wasn't
		}
		else {
			sendError(args, 462, client);
			if (DEBUG)
				std::cout << RED << "Client FD" << client->getFd() << "is already registered" << ENDCOLOR << std::endl;
			return (true); 
		}
	}
	else if (cmd == "PING") {
		std::string tocken = args.substr(args.find(':') + 1); //token should be the same as recieved by client
		std::string msg = ":server PONG " + tocken + "\r\n";
		send(client->getFd(), msg.c_str(), msg.size(), 0);
		if (DEBUG)
			std::cout << GREEN << "PING command was accepted and answered" << ENDCOLOR << std::endl;
	}
	return (true);
}

void Server::sendError(std::string args, int errorNumber, Client* client) {
	std::string err;
	if (errorNumber == 433)
		err = ":server 433 * " + args + " :Nickname is already in use\r\n";
	else if (errorNumber == 432)
		err = ":server 432 * " + args + " :Erroneus nickname\r\n";
	else if (errorNumber == 464)
		err = ":server 464 * :Password incorrect\r\n";
	else if (errorNumber == 461)
		err = ":server 461 USER :Not enough parameters\r\n";
	else if (errorNumber == 462)
		err = ":server 462 * :You may not reregister\r\n";
	
	send(client->getFd(), err.c_str(), err.size(), 0);
	if (DEBUG)
		std::cout << PINK << "Error " << errorNumber << " for client FD " << client->getFd() << " has been sent!" << ENDCOLOR << std::endl;
}

void Server::registerClient(Client& client) {
	if (!client.getRegistered()) {
		client.setRegistered(); //Ira: registered if all set, if not return false in next if condition, all messages about errors have been sent before
		std::string msg = ":server 001 " + client.getNickname() + " :Welcome to the IRC Network, " + client.getNickname() + "\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		if (DEBUG) {
			std::cout << GREEN << "Client FD " << client.getFd() << " has been registered" << ENDCOLOR << std::endl;
			std::cout << client << std::endl;
		}
	}
}