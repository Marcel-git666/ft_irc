#ifndef SERVER_HPP
#define SERVER_HPP

#define RED "\033[31m"
#define GREEN "\033[32m"
#define PINK "\033[35m"
#define BLUE "\033[36m"
#define ENDCOLOR "\033[0m"
#define DEBUG true

#include "Client.hpp"
#include <map>
#include <poll.h>
#include <string>
#include <vector>

class Server {
private:
  int _port;
  std::string _password;
  int _serverSocketFd;
  std::vector<struct pollfd> _fds; // The list of file descriptors for poll()
  std::map<int, Client *> _clients;

  void init();
  void acceptNewClient();
  void disconnectClient(int fd); //Ira, for removing when password is wrong
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
  bool checkNickname(std::string arg); //for nickname uniqness
  void setClientInfo(std::string args, Client* client); //fset username and realname
  
};

#endif
