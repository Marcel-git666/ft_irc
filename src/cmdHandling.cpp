#include "../inc/Server.hpp"

/**
 * @brief Parses the raw message, extracting the command and leaving the rest in
 * args.
 * * @param args The full raw message string (e.g., "JOIN #test"). It is
 * modified in-place to contain only the arguments after the command.
 * @return std::string The extracted command (e.g., "JOIN").
 */
std::string Server::extractCMD(std::string &args) {
  size_t pos = args.find(' ');

  // If there is no space, the whole string is the command (e.g., just "QUIT")
  if (pos == std::string::npos) {
    std::string command = args;
    args = ""; // No arguments left
    if (DEBUG)
      std::cout << "Command from client: " << command << " (No args)\n";
    return command;
  }

  std::string command = args.substr(0, pos);
  args = args.substr(pos + 1); // Keep everything after the first space as args

  if (DEBUG) {
    std::cout << "Command from client: " << command << " | ARGS: " << args
              << std::endl;
  }
  return command;
}

/**
 * @brief The main dispatcher for all incoming IRC commands.
 * * @param args The full message received from the client.
 * @param client The client who sent the message.
 * @return true if the command was processed, false if the connection should be
 * dropped (e.g., wrong password).
 */
bool Server::executeCMD(std::string &args, Client &client) {
  std::string cmd = extractCMD(args);
  if (cmd.empty())
    return false;

  // --- AUTHENTICATION PHASE ---
  if (cmd == "PASS") {
    if (!_password.empty() && args != _password) {
      if (DEBUG)
        std::cout << RED << "Password doesn't match" << ENDCOLOR << std::endl;
      sendError(args, 464, client); // ERR_PASSWDMISMATCH
      return false;                 // Immediately disconnect client
    }
    client.setHasPassword();
    if (DEBUG)
      std::cout << GREEN << "Client FD " << client.getFd()
                << " entered correct password\n"
                << ENDCOLOR;

  } else if (cmd == "NICK") {
    std::string checkNick = checkNickname(args);
    if (checkNick != "ok") {
      if (checkNick == "433")
        sendError(args, 433, client); // ERR_NICKNAMEINUSE
      else if (checkNick == "432")
        sendError(args, 432, client); // ERR_ERRONEUSNICKNAME
      return true;
    }

    // If client already has a nick, inform others of the change
    if (!client.getNickname().empty()) {
      std::string oldNick = client.getNickname();
      sendMsgToClient(":" + oldNick + "!" + client.getUsername() +
                          "@localhost NICK :" + args + "\r\n",
                      client);
      if (DEBUG)
        std::cout << GREEN << "Client FD " << client.getFd()
                  << " changed nick to: " << args << ENDCOLOR << "\n";
    } else {
      if (DEBUG)
        std::cout << GREEN << "New Client FD " << client.getFd()
                  << " nick set to: " << args << ENDCOLOR << "\n";
    }
    client.setNickname(args);

    if (!client.getRegistered()) {
      if (!registerClient(client))
        return false;
    }

  } else if (cmd == "USER") {
    if (!client.getRegistered()) {
      size_t spacePos = args.find(' ');
      if (spacePos == std::string::npos) {
        sendError("USER", 461, client); // ERR_NEEDMOREPARAMS
        return true;
      }

      std::string username = args.substr(0, spacePos);
      if (username.length() > USERLEN) {
        username = username.substr(0, USERLEN); // Truncate to limit
      }
      if (username[0] != '~') {
        username.insert(0, 1, '~'); // IRC standard prefix
      }
      client.setUsername(username);

      size_t colonPos = args.find(':');
      std::string realname = (colonPos != std::string::npos)
                                 ? args.substr(colonPos + 1)
                                 : username;
      client.setRealname(realname);

      if (!registerClient(client))
        return false;
    } else {
      sendError(args, 462, client); // ERR_ALREADYREGISTRED
      return true;
    }

    // --- GENERAL COMMANDS ---
  } else if (cmd == "PING") {
    size_t colonPos = args.find(':');
    std::string token =
        (colonPos != std::string::npos) ? args.substr(colonPos + 1) : args;
    sendPong(client, token);
    if (DEBUG)
      std::cout << GREEN << "PING answered\n" << ENDCOLOR;

  } else if (cmd == "QUIT") {
    disconnectClient(client.getFd(), args);

    // --- COMMANDS REQUIRING REGISTRATION ---
  } else {
    if (client.getRegistered()) {
      if (cmd == "PRIVMSG") {
        if (args[0] != '#')
          sendPrivateMsg(client, args);
        else
          sendToChannel(client, args);
      } else if (cmd == "JOIN") {
        if (args[0] != '#')
          sendError(args, 403, client);
        else
          joinChannel(&client, args);
      } else if (cmd == "INVITE") {
        inviteToChan(client, args);
      } else if (cmd == "KICK") {
        kickOutOfChannel(client, args);
      } else if (cmd == "MODE") {
        if (args[0] != '#')
          sendError(args, 403, client);
        else
          operateMode(client, args);
      } else if (cmd == "TOPIC") {
        if (args[0] != '#')
          sendError(args, 403, client);
        else
          setTopic(client, args);
      } else if (cmd == "PART") {
        if (args[0] != '#')
          sendError(args, 403, client);
        else
          execPART(client, args);
      }
    }
  }
  return true;
}

// --- UTILITY SENDERS & SEARCHERS ---

/**
 * @brief Sends a PONG response back to the client to keep the connection alive.
 */
void Server::sendPong(const Client &client, const std::string &token) {
  sendMsgToClient(":server PONG " + token + "\r\n", client);
}

/**
 * @brief Sends a formatted error message to the client.
 */
void Server::sendError(const std::string &args, int errorNumber,
                       const Client &client) {
  std::string err = GetErrorStr(args, errorNumber);
  client.sendMessage(err);
  if (DEBUG) {
    std::cout << PINK << "Error " << errorNumber << " sent to FD "
              << client.getFd() << ENDCOLOR << std::endl;
  }
}

/**
 * @brief Wraps the client's sendMessage method, adding debug output.
 */
void Server::sendMsgToClient(const std::string &msg, const Client &client) {
  client.sendMessage(msg);
  if (DEBUG) {
    std::cout << GREEN << "Sent to FD " << client.getFd() << ": " << msg
              << ENDCOLOR;
  }
}

/**
 * @brief Looks up a client's File Descriptor by their nickname.
 * @return The FD if found, or -1 if no such client exists.
 */
int Server::clientFdsearch(const std::string &nickName) {
  if (nickName.empty())
    return -1;
  for (std::map<int, Client *>::iterator it = _clients.begin();
       it != _clients.end(); ++it) {
    if (it->second->getNickname() == nickName) {
      return it->first;
    }
  }
  return -1;
}

/**
 * @brief Retrieves a Client pointer by their File Descriptor.
 * @return Pointer to the Client, or NULL if not found.
 */
Client *Server::findClient(int clientFD) {
  std::map<int, Client *>::iterator it = _clients.find(clientFD);
  if (it != _clients.end()) {
    return it->second;
  }
  return NULL;
}
