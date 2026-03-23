#include "Bot.hpp"
#include <csignal>
#include <cstdlib>
#include <iostream>

Bot *globalBot = NULL;

// Zpracování Ctrl+C pro bota
void signalHandler(int signum) {
  (void)signum;
  std::cout << "\nUkoncuji bota..." << std::endl;
  if (globalBot) {
    globalBot->stop();
  }
}

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Pouziti: ./ircbot <IP> <port> <heslo>" << std::endl;
    return 1;
  }

  signal(SIGINT, signalHandler);

  std::string ip = argv[1];
  int port = std::atoi(argv[2]);
  std::string password = argv[3];

  try {
    Bot myBot(ip, port, password);
    globalBot = &myBot;

    myBot.run(); // Zde se program "zasekne" v nekonečné smyčce

  } catch (const std::exception &e) {
    std::cerr << "Fatalni chyba bota: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
