#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include "Definitions.hpp" //Ira: I put defines in a different file to keep this hpp more clear
#include <arpa/inet.h> // for inet_ntoa
#include <cctype>      // Ira: for isdigit nickname checking
#include <cerrno>
#include <cstring> // <--- REQUIRED for std::memset
#include <ctime>   // Ira: time of server creation
#include <fcntl.h> // <--- REQUIRED for fcntl, F_SETFL, O_NONBLOCK
#include <iostream>
#include <map>
#include <netinet/in.h> // Required for sockaddr_in
#include <poll.h>
#include <sstream> // Ira: For reading from string, split targets in a private message
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h> // for close()
#include <vector>
#include <csignal>

class Server {
private:
  bool _is_running;
  int _port;
  std::string _password;
  int _serverSocketFd;
  std::string _creationTime;
  std::vector<struct pollfd> _fds; // The list of file descriptors for poll()
  std::map<int, Client *> _clients;
  std::map<std::string, Channel> _channels;

  void init();
  void acceptNewClient();
  void
  disconnectClient(int fd,
                   std::string args); // Ira: for removing when password is
                                      // wrong, args are needed if Client quit
                                      // by himself and send a reaason why
  void
  sendMsgToClient(const std::string &msg,
                  const Client &client); // Ira: sending message to the client
  void
  sendError(const std::string &args, int errorNumber,
            const Client &client); // Ira: sending error message to the client
  std::string
  GetErrorStr(const std::string &args,
              int errorNumber); // generate Error message by Error number
  void sendPing(
      const Client &client); // Ira: don't know if we need it, can be deleted
  void sendPong(
      const Client &client,
      const std::string &token); // Ira: for answering Clients PING if it comes
  // OCF - Private to prevent copying
  Server(const Server &other);
  Server &operator=(const Server &other);

public:
  Server(int port, const std::string &password);
  ~Server();
  void run();
  void cleanMemory();

  // Ira: parcer
  std::string extractCMD(std::string &args);

  // Ira: executer
  bool executeCMD(std::string &args,
                  Client &client); // return true if everithing ok

  // Ira: utils
  std::string checkNickname(const std::string &arg); // for nickname uniqness
  int clientFdsearch(const std::string &nickName);   // get FD from nickname
  Client *findClient(int clientFD);

  bool registerClient(Client &client);
  void sendPrivateMsg(const Client &client, const std::string &args);

  // Ira: channel operations
  void joinChannel(Client *client, std::string &args);
  void connectToChannel(Client *client, const std::string &channelName,
                        const std::string &key);
  void kickOutOfChannel(Client &client, const std::string &args);
  void inviteToChan(Client &sender, const std::string &args);
  void operateMode(Client &sender, const std::string &args);
  void applyMode(Client &sender, Channel *chan, const std::string &modestring);
  void sendChanMode(Client &sender, Channel *chan);
  void setTopic(Client &client, const std::string &args);
  void execPART(Client &sender, const std::string &args);
  void broadcastChannel(Channel *ch, const std::string &command,
                        const std::string &topic, Client &sender);

  // Ira: utils for channelCMD
  Channel *searchChannel(const std::string &name);
  void sendToChannel(Client &sender, const std::string &args);
  void sendNames(Channel &ch, Client *client);
  std::vector<std::string> split(const std::string &targetsStr, char delimeter);
  void sendMsgToFd(int fd, const std::string &msg);

  // Ira: for cleaning when client disconnecting
  void deleteClientFromChannels(int FD, const std::string &reason);

  // Ira: for tester need to add Clients without sockets
  void addClient(Client &client);
};

#endif
