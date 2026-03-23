#include "Bot.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

Bot::Bot(const std::string &ip, int port, const std::string &password)
    : _ip(ip), _port(port), _password(password), _nickname("MrBot"),
      _socketFd(-1), _isRunning(true) {}

Bot::~Bot() {
  if (_socketFd != -1) {
    close(_socketFd);
    std::cout << "Bot has finished, socket is closed." << std::endl;
  }
}

void Bot::connectToServer() {
  // AF_INET = IPv4, SOCK_STREAM = TCP
  _socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (_socketFd == -1) {
    throw std::runtime_error("Error: Failed to create socket");
  }
  struct sockaddr_in addr;

  std::memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET; // IPv4
  addr.sin_port = htons(_port);
  addr.sin_addr.s_addr = inet_addr(_ip.c_str());

  if (connect(_socketFd, reinterpret_cast<struct sockaddr *>(&addr),
              sizeof(addr)) == -1) {
    throw std::runtime_error("Error: failed to connect");
  }
}

void Bot::sendMessage(const std::string &msg) {
  std::string toSend = msg + "\r\n";
  if (send(_socketFd, toSend.c_str(), toSend.size(), 0) < 0) {
    throw std::runtime_error("Error: failed to send message");
  }
}

void Bot::authenticate() {
  sendMessage("PASS " + _password);
  sendMessage("NICK " + _nickname);
  sendMessage("USER bot 0 * :I am a joke bot");
}

void Bot::run() {
  connectToServer();
  authenticate();
  listenLoop();
}

void Bot::processMessage(const std::string &msg) {
  if (msg.find("PRIVMSG") != std::string::npos) {
    size_t pos1 = msg.find(":");
    size_t pos2 = msg.find("!");
    std::string sender;
    if (pos1 != std::string::npos && pos2 != std::string::npos) {
      sender = msg.substr(pos1 + 1, pos2 - pos1 - 1);
    }
    if (msg.find("!joke") != std::string::npos) {
      sendJoke(sender);
    } else if (msg.find("!help") != std::string::npos) {
      sendHelp(sender);
    }
  }
}

void Bot::sendJoke(const std::string &target) {
  sendMessage("PRIVMSG " + target + " :My joke.");
}

void Bot::sendHelp(const std::string &target) {
  sendMessage("PRIVMSG " + target + " :Really helpful message.");
}

void Bot::stop() { _isRunning = false; }

void Bot::listenLoop() {
  while (_isRunning) {
    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));

    // Bot čeká (blokuje se), dokud server něco nepošle
    int bytes = recv(_socketFd, buffer, sizeof(buffer) - 1, 0);

    // Pokud recv vrátí 0 (server se odpojil) nebo -1 (chyba/ukončení přes
    // Ctrl+C)
    if (bytes <= 0) {
      std::cout << "\nSpojeni se serverem bylo ukonceno." << std::endl;
      _isRunning = false;
      break;
    }

    std::string text = buffer;

    // Vypsání do terminálu bota pro kontrolu
    std::cout << "Slysim: " << text;

    // Zpracování zprávy (tvoje logika s !joke a !help)
    processMessage(text);
  }
}
