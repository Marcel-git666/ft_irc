#include "../inc/Server.hpp"

// Ira: execute commands
bool Server::executeCMD(std::string &args, Client &client) {
  std::string cmd = extractCMD(args);
  if (cmd.empty())
    return (false);
  if (cmd == "PASS") {
    if (args != _password && !_password.empty()) {
      if (DEBUG)
        std::cout << RED << "Password doesn't match" << ENDCOLOR << std::endl;
      sendError(args, 464, client);
      return (
          false); // Ira: returned here because if password is wrong we don't
                  // accept anything else and immediately disconnect this client
    }
    client.setHasPassword();
    if (DEBUG)
      std::cout << GREEN << "Client FD " << client.getFd()
                << " has input correct password" << ENDCOLOR << std::endl;
  } else if (cmd == "NICK") {
    std::string checkNick = checkNickname(args);
    if (checkNick != "ok") {
      if (checkNick == "433")
        sendError(args, 433, client);
      else if (checkNick == "432")
        sendError(args, 432, client);
      return (true);
    } else {
      if (client.getNickname() != "") {
        // [TO DO] (if NICK changes, need to send info to other users in
        // channels)
        std::string oldNick = client.getNickname();
        sendMsgToClient(":" + oldNick + "!" + client.getUsername() +
                            "@localhost NICK :" + args + "\r\n",
                        client);
        if (DEBUG)
          std::cout << GREEN << "Nickname of client FD " << client.getFd()
                    << " changed to: " << args << ENDCOLOR << std::endl;
      } else {
        if (DEBUG)
          std::cout << GREEN << "Nickname of new client FD " << client.getFd()
                    << ": " << args << ENDCOLOR << std::endl;
      }
      client.setNickname(args);
    }
    if (!client.getRegistered())
      if (!registerClient(client))
        return (false);
  } else if (cmd == "USER") {
    if (!client.getRegistered()) {
      // Ira: USER command looks like USER <username> 0 * :<realname>
      // clients usually use for username and realname the nickname, if it's not
      // set by user to expect an empty username is an extra safety, but we have
      // error massage for it so I included it
      std::string username = args.substr(0, args.find(' ')); // find first arg
      if (username.empty())
        sendError("USER", 461, client);
      if (username.length() > USERLEN) {
        username = username.substr(0, USERLEN);
        // Ira: IRCstandard: "If this length is advertised, the username
        // MUST be silently truncated to the given length before being used."
      }
      if (username[0] != '~')
        username.insert(0, 1, '~');
      // Ira: IRCstandard: "... the username provided by the client SHOULD be
      // prefixed
      //  by a tilde ('~', 0x7E) to show that this value is user-set."
      client.setUsername(username);
      if (DEBUG)
        std::cout << GREEN << "Client FD " << client.getFd() << " username is "
                  << client.getUsername() << ENDCOLOR << std::endl;
      std::string realname = args.substr(args.find(':') + 1);
      client.setRealname(realname);
      if (DEBUG)
        std::cout << GREEN << "Client FD " << client.getFd() << " realname is "
                  << client.getRealname() << ENDCOLOR << std::endl;
      if (!registerClient(client)) // Ira: registration if wasn't
        return (false); // we need tu return smth for the case when client can't
                        // be registere because of password absence
    } else {
      sendError(args, 462, client);
      if (DEBUG)
        std::cout << RED << "Client FD" << client.getFd()
                  << "is already registered" << ENDCOLOR << std::endl;
      return (true);
    }
  } else if (cmd == "PING") {
    std::string token = args.substr(
        args.find(':') + 1); // token should be the same as recieved by client
    //[ToDo] From Copilot appeared that it's not obligated to have ":", whithout
    // it also should work
    sendPong(client, token);
    if (DEBUG)
      std::cout << GREEN << "PING command was accepted and answered" << ENDCOLOR
                << std::endl;
  } else if (cmd == "QUIT")
    disconnectClient(client.getFd(), args);
  else {
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
        // Ira: when client close the channel not "/quit" server it sends PART
        // command,
        //  also you can do /part #channel1,#channel2,#channel3... :reason from
        //  client side, but from inside the channel you can only do /close
        //  (reason automaticaly generated)
        if (args[0] != '#')
          sendError(args, 403, client);
        else
          execPART(client, args);
      }
    }
  }
  return (true);
}

// Ira: extract command, and cut the message to the args
std::string Server::extractCMD(std::string &args) {
  size_t pos = args.find(' ');
  if (pos == std::string::npos)
    return ("");
  std::string command = args.substr(0, pos);
  if (DEBUG)
    std::cout << "Command from client: " << command;
  args = args.substr(pos + 1, args.length());
  if (DEBUG)
    std::cout << " ARGS for this command: " << args << "." << std::endl;
  return command;
}

void Server::sendPong(const Client &client, const std::string &token) {
  sendMsgToClient(":server PONG " + token + "\r\n", client);
}

void Server::sendError(const std::string &args, int errorNumber,
                       const Client &client) {
  std::string err = GetErrorStr(args, errorNumber);
  client.sendMessage(err);
  if (DEBUG)
    std::cout << PINK << "Error " << errorNumber << " for client FD "
              << client.getFd() << " has been sent!" << ENDCOLOR << std::endl;
}

void Server::sendMsgToClient(const std::string &msg, const Client &client) {
  client.sendMessage(msg);
  if (DEBUG)
    std::cout << GREEN << "Message " << msg << "' from server from client FD "
              << client.getFd() << " has been sent!" << ENDCOLOR << std::endl;
}

int Server::clientFdsearch(const std::string &nickName) {
  if (nickName != "") {
    for (std::map<int, Client *>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
      if ((it->second)->getNickname() == nickName)
        return it->first; // return the fd (the key)
    }
  }
  return (-1);
}

Client *Server::findClient(int clientFD) {
  for (std::map<int, Client *>::iterator it = _clients.begin();
       it != _clients.end(); it++) {
    if (it->first == clientFD)
      return (it->second);
  }
  return (NULL);
}
