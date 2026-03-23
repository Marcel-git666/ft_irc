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

  // OCF - Zakázání kopírování (Bot by se neměl kopírovat kvůli socketu)
  Bot(const Bot &other);
  Bot &operator=(const Bot &other);

  // Interní síťové metody
  void connectToServer();
  void authenticate();
  void listenLoop();
  void sendMessage(const std::string &msg);

  // Mozek bota (zpracování textu)
  void processMessage(const std::string &msg);
  void sendJoke(const std::string &target);
  void sendHelp(const std::string &target);

public:
  // Konstruktor a Destruktor
  Bot(const std::string &ip, int port, const std::string &password);
  ~Bot();

  // Hlavní spouštěcí metoda
  void run();
  void stop();
};

#endif
