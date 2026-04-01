#include "Bot.hpp"
#include <csignal>
#include <cstdlib>
#include <iostream>

volatile sig_atomic_t g_signaled = 0;

// Handles the SIGINT (Ctrl+C) signal to safely terminate the bot.
void signalHandler(int signum) {
  (void)signum;
  g_signaled = 1;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./ircbot <IP> <port> <password>" << std::endl;
    return 1;
  }

  struct sigaction sa;
  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  std::string ip = argv[1];
  int port = std::atoi(argv[2]);
  std::string password = argv[3];

  try {
    Bot myBot(ip, port, password);
    myBot.run();

  } catch (const std::exception &e) {
    std::cerr << "Fatal bot error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
