#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <map>
#include <poll.h>
#include <string>
#include <vector>
#include "Definitions.hpp" //Ira: I put defines in a different file to keep this hpp more clear

class Server {
private:
  int _port;
  std::string _password;
  int _serverSocketFd;
  std::vector<struct pollfd> _fds; // The list of file descriptors for poll()
  std::map<int, Client *> _clients;

  void init();
  void acceptNewClient();
  void disconnectClient(int fd); //Ira: for removing when password is wrong
  void sendError(const std::string args, int errorNumber, Client* client); //Ira: sending errors to the client
  void sendPing(Client* client); //Tra: to keep client alive (connected)
  void sendPong(Client* Client, std::string args);
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
  bool executeCMD(std::string cmd, std::string args, Client* client); //return true if everithing ok

  //Ira: utils
  std::string checkNickname(std::string arg); //for nickname uniqness

  
};

#endif
