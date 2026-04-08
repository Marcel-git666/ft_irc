#include "../inc/Server.hpp"

/**
 * @brief Validates a proposed nickname for uniqueness and allowed characters.
 * * @param arg The requested nickname.
 * @return std::string "ok" if valid, "433" if already in use, "432" if invalid
 * format.
 */
std::string Server::checkNickname(const std::string &arg) {
  if (arg.empty())
    return "432"; // ERR_ERRONEUSNICKNAME

  // 1. Check uniqueness among all currently connected clients
  for (std::map<int, Client *>::iterator it = _clients.begin();
       it != _clients.end(); ++it) {
    if (it->second && it->second->getNickname() == arg)
      return "433"; // ERR_NICKNAMEINUSE
  }

  // 2. Check rules: cannot start with digit, no spaces, colons, or hashtags
  if (isdigit(arg[0]) || arg.find(':') != std::string::npos ||
      arg.find('#') != std::string::npos ||
      arg.find(' ') != std::string::npos) {
    return "432"; // ERR_ERRONEUSNICKNAME
  }

  return "ok";
}

/**
 * @brief Attempts to fully register a client.
 * * Checks if the client has provided a valid password, nickname, and user
 * info. If successful, sends the standard IRC welcome messages (001-004).
 * * @param client The client attempting to register.
 * @return true if registration logic executed without critical errors.
 */
bool Server::registerClient(Client &client) {
  // Check server password if one is required
  if (!_password.empty() && !client.getHasPassword()) {
    sendError(client.getNickname(), 464, client);
    return false;
  }

  // setRegistered() returns true only if NICK, USER, and PASS are all
  // successfully set
  if (client.setRegistered()) {
    // 001: Welcome message
    std::string msg = ":server 001 " + client.getNickname() +
                      " :Welcome to the IRC Network, " + client.getNickname() +
                      "\r\n";
    sendMsgToClient(msg, client);

    // 002: Your host
    msg = ":server 002 " + client.getNickname() +
          " :Your host is 42_ircserv, running version 01\r\n";
    sendMsgToClient(msg, client);

    // 003: Created
    msg = ":server 003 " + client.getNickname() + " :This server was created " +
          _creationTime + "\r\n";
    sendMsgToClient(msg, client);

    // 004: Server Info (Format: <nick> <servername> <version> <usermodes>
    // <channelmodes>) Channel modes supported: i (invite), t (topic
    // restricted), k (key), o (oper), l (limit)
    msg = ":server 004 " + client.getNickname() + " 42_ircserv 1.0 o itkol\r\n";
    sendMsgToClient(msg, client);

    if (DEBUG) {
      std::cout << GREEN << "Client FD " << client.getFd()
                << " has been registered" << ENDCOLOR << std::endl;
      std::cout << client << std::endl;
    }
  } else {
    // Client is missing something (e.g., they sent PASS and NICK, but not USER
    // yet)
    if (DEBUG) {
      std::cout << RED << "Client FD " << client.getFd()
                << " is missing information and couldn't be registered yet."
                << ENDCOLOR << std::endl;
    }
  }
  return true;
}
