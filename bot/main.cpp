#include "Bot.hpp"
#include <csignal>
#include <cstdlib>
#include <iostream>

Bot *globalBot = NULL;

// Handles the SIGINT (Ctrl+C) signal to safely terminate the bot.
void signalHandler(int signum) {
  (void)signum;
  std::cout << std::endl;
  if (globalBot) {
    globalBot->stop();
  }
}

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./ircbot <IP> <port> <password>" << std::endl;
    return 1;
  }

  signal(SIGINT, signalHandler);

  std::string ip = argv[1];
  int port = std::atoi(argv[2]);
  std::string password = argv[3];

  try {
    Bot myBot(ip, port, password);
    globalBot = &myBot;

    myBot.run();

  } catch (const std::exception &e) {
    std::cerr << "Fatal bot error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
