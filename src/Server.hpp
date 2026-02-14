#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <map>
#include <poll.h>
#include <string>
#include <vector>
#include "Definitions.hpp" //Ira: I put defines in a different file to keep this hpp more clear
#include <ctime> // Ira: time of server creation
#include <sstream> // Ira: For reading from string, split targets in a private message
#include <arpa/inet.h> // for inet_ntoa
#include <cstring>     // <--- REQUIRED for std::memset
#include <fcntl.h>     // <--- REQUIRED for fcntl, F_SETFL, O_NONBLOCK
#include <iostream>
#include <netinet/in.h> // Required for sockaddr_in
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h> // for close()
#include <cctype> // Ira: for isdigit nickname checking

class Server {
private:
  int _port;
  std::string _password;
  int _serverSocketFd;
  std::string _creationTime;
  std::vector<struct pollfd> _fds; // The list of file descriptors for poll()
  std::map<int, Client *> _clients;
  std::map<std::string, Channel> _channels;


  void init();
  void acceptNewClient();
  void disconnectClient(int fd); //Ira: for removing when password is wrong
  void sendMsgToClient(std::string msg, const Client& client); //Ira: sending message to the client
  void sendError(std::string args, int errorNumber, const Client& client); //Ira: sending error message to the client
  std::string GetErrorStr(std::string args, int errorNumber, const Client& client); //generate Error message by Error number
  void sendPing(const Client& client); //Ira: don't know if we need it, can be deleted
  void sendPong(const Client& client, std::string token); //Ira: for answering Clients PING if it comes
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
  bool executeCMD(std::string cmd, std::string args, Client& client); //return true if everithing ok

  //Ira: utils
  std::string checkNickname(std::string arg); //for nickname uniqness
  int clientFdsearch(std::string nickName); //get FD from nickname

  bool registerClient(Client& client);
  void sendPrivateMsg(const Client& client, std::string args);

  //channel operations
  void createNewChannel(Client* client, std::string& name);
  void connectToChannel(Client* client, std::string& channelName);
  Channel* searchChannel(const std::string& name);

};

#endif
