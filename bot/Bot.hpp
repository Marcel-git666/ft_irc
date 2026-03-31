#ifndef BOT_HPP
#define BOT_HPP

#include <string>

class Bot {
private:
  std::string _ip;
  int _port;
  std::string _password;
  std::string _nickname;
  int _socketFd;
  bool _isRunning;

  // OCF - Disable copying (Bot should not be copied to prevent double socket
  // closing)
  Bot(const Bot &other);
  Bot &operator=(const Bot &other);

  // Internal network operations
  void connectToServer();
  void authenticate();
  void listenLoop();
  void sendMessage(const std::string &msg);

  // Bot logic and message processing
  void processMessage(const std::string &msg);
  void sendJoke(const std::string &target);
  void sendHelp(const std::string &target);

public:
  // Constructor and Destructor
  Bot(const std::string &ip, int port, const std::string &password);
  ~Bot();

  // Main control methods
  void run();
  void stop();
};

#endif
