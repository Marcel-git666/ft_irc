#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <string>
#include <vector>

class Server {
private:
  int _port;
  std::string _password;
  int _serverSocketFd;
  std::vector<struct pollfd> _fds; // The list of file descriptors for poll()

  void init();
  // OCF - Private to prevent copying
  Server(const Server &other);
  Server &operator=(const Server &other);

public:
  Server(int port, std::string password);
  ~Server();
  void run();
};

#endif
