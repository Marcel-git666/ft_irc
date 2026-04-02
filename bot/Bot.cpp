#include "Bot.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

extern volatile sig_atomic_t g_signaled;
// Initializes the Bot with given IP, port, and password. Seeds the random
// number generator.
// Establishes a TCP connection to the specified IRC server.
Bot::Bot(const std::string &ip, int port, const std::string &password)
    : _ip(ip), _port(port), _password(password), _nickname("MrBot"),
      _isRunning(true), _buffer("") {
  std::srand(std::time(NULL));
  _socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (_socketFd == -1) {
    throw std::runtime_error("Error: Failed to create socket");
  }

  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(_port);
  addr.sin_addr.s_addr = inet_addr(_ip.c_str());

  if (connect(_socketFd, reinterpret_cast<struct sockaddr *>(&addr),
              sizeof(addr)) == -1) {
    close(_socketFd);
    throw std::runtime_error("Error: Failed to connect to server");
  }
}

// Safely closes the socket when the Bot instance is destroyed.
Bot::~Bot() {
  if (_socketFd != -1) {
    close(_socketFd);
    std::cout << "Bot has finished, socket is closed." << std::endl;
  }
}

// Appends the required IRC line ending (\r\n) and sends the message to the
// server.
void Bot::sendMessage(const std::string &msg) {
  std::string toSend = msg + "\r\n";
  if (send(_socketFd, toSend.c_str(), toSend.size(), 0) < 0) {
    throw std::runtime_error("Error: Failed to send message");
  }
}

// Sends authentication details (PASS, NICK, USER) to register on the IRC
// network.
void Bot::authenticate() {
  sendMessage("PASS " + _password);
  sendMessage("NICK " + _nickname);
  sendMessage("USER bot 0 * :I am a joke bot");
}

// Main execution flow: connects, authenticates, and starts listening.
void Bot::run() {
  authenticate();
  listenLoop();
}

// Parses incoming messages to find commands like !joke or !help and extracts
// the sender's name.
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
  } else if (msg.find("PING") != std::string::npos) {
    size_t pos = msg.find(":");
    if (pos != std::string::npos) {
      sendMessage("PONG :" + msg.substr(pos + 1));
    }
  }
}

// Selects a random joke from an array and sends it to the target user.
void Bot::sendJoke(const std::string &target) {
  static const std::string jokes[5] = {
      "Why do programmers prefer dark mode? Because light attracts bugs.",
      "Chuck Norris's keyboard doesn't have a Ctrl key because nothing "
      "controls Chuck Norris.",
      "Why don't scientists trust atoms? Because they make up everything.",
      "What do you call a fake noodle? An impasta.",
      "I told my wife she was drawing her eyebrows too high. She looked "
      "surprised."};

  int randomIndex = std::rand() % 5;
  sendMessage("PRIVMSG " + target + " :" + jokes[randomIndex]);
}

// Sends a descriptive help message explaining the bot's capabilities.
void Bot::sendHelp(const std::string &target) {
  sendMessage("PRIVMSG " + target +
              " :Hi, I am MrBot! Available commands: "
              "!joke (gives you a random joke), "
              "!help (shows this message). "
              "I am a simple bot, but I do my best!");
}

// Infinite loop that continuously waits for and reads data from the server.
void Bot::listenLoop() {
  while (_isRunning) {
    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));

    int bytes = recv(_socketFd, buffer, sizeof(buffer) - 1, 0);

    if (bytes <= 0) {
      if (g_signaled) {
        std::cout << "\nShutting down MrBot (Ctrl+C detected)..." << std::endl;
      } else {
        std::cout << "Connection with server has been shut down." << std::endl;
      }
      _isRunning = false;
      break;
    }
    _buffer += buffer;
    size_t pos;
    while ((pos = _buffer.find("\r\n")) != std::string::npos) {
      std::string singleMessage = _buffer.substr(0, pos);
      _buffer.erase(0, pos + 2);
      std::cout << "Listening: " << singleMessage << std::endl;
      processMessage(singleMessage);
    }
  }
}
