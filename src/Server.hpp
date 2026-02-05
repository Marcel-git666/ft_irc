#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <map>
#include <poll.h>
#include <string>
#include <vector>

#define DEBUG true

class Server {
private:
  int _port;
  std::string _password;
  int _serverSocketFd;
  std::vector<struct pollfd> _fds; // The list of file descriptors for poll()
  std::map<int, Client *> _clients;

  void init();
  void acceptNewClient();
  // OCF - Private to prevent copying
  Server(const Server &other);
  Server &operator=(const Server &other);

public:
  Server(int port, std::string password);
  ~Server();
  void run();


  //Ira: parcer
  std::string extractCMD(std::string& args);

  //Ira: executer
  void executeCMD(std::string cmd, std::string args, Client* client);

  //Ira: utils
  bool comparePassword(std::string arg); // for password checking
  void setClientInfo(std::string args, Client* client); //fset username and realname
};

#endif
