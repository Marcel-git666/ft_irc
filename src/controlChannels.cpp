#include "../inc/Server.hpp"
#include <vector>

/**
 * @brief Helper function to connect a specific client to a channel.
 * Creates the channel if it doesn't exist, or checks modes (+k, +i, +l) if it
 * does.
 * * @param client Pointer to the client attempting to join.
 * @param name The name of the channel (e.g., "#general").
 * @param key The password provided by the client (if any).
 */
void Server::connectToChannel(Client *client, const std::string &name,
                              const std::string &key) {
  Channel *ch = searchChannel(name);

  if (!ch) {
    // Channel does not exist, create it and insert into the map
    std::pair<std::map<std::string, Channel>::iterator, bool> result;
    result = _channels.insert(std::make_pair(name, Channel(client, name)));
    ch = &result.first->second; // Safe pointer to the newly created channel
  } else {
    // Channel exists, validate constraints (Modes: +k, +i, +l)
    if (ch->getKeySetting() && key != ch->getKey()) {
      sendError(ch->getChName(), 475, *client); // ERR_BADCHANNELKEY
      return;
    }
    if (ch->getInviteSettings() && !ch->userInvited(client->getFd())) {
      sendError(ch->getChName(), 473, *client); // ERR_INVITEONLYCHAN
      return;
    }
    if (ch->isFull()) {
      sendError(ch->getChName(), 471, *client); // ERR_CHANNELISFULL
      return;
    }

    // Validated, add user to the channel
    ch->addMember(client->getFd(), client->getNickname());
    if (ch->getInviteSettings()) {
      ch->removeFromInvited(client->getFd());
    }
  }

  // Broadcast JOIN message to all members in the channel
  std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
                    "@localhost JOIN " + name + "\r\n";

  std::map<int, std::string> members = ch->getMembers();
  for (std::map<int, std::string>::iterator it = members.begin();
       it != members.end(); ++it) {
    sendMsgToFd(it->first, msg);
  }

  // Send the channel TOPIC to the new user
  std::string topic = ch->getTopic();
  if (!topic.empty()) {
    msg = ":server 332 " + client->getNickname() + " " + name + " :" + topic +
          "\r\n";
  } else {
    msg = ":server 331 " + client->getNickname() + " " + name +
          " :No topic is set\r\n";
  }
  sendMsgToClient(msg, *client);

  // Send the list of current members (NAMES) to the new user
  sendNames(*ch, client);
}

/**
 * @brief Handles the JOIN command, allowing connection to multiple channels.
 * Parses the target channels and keys (e.g., "JOIN #chan1,#chan2 key1").
 */
void Server::joinChannel(Client *client, std::string &args) {
  std::vector<std::string> targets;
  std::string key = "";

  size_t spacePos = args.find(' ');
  if (spacePos != std::string::npos) {
    key = args.substr(spacePos + 1);
    args.erase(
        spacePos); // Remove the key part from args, leaving only channels
  }

  size_t commaPos = args.find(',');
  if (commaPos != std::string::npos) {
    targets = split(args, ',');
  } else {
    targets.push_back(args);
  }

  for (std::vector<std::string>::iterator it = targets.begin();
       it != targets.end(); ++it) {
    connectToChannel(client, *it, key);
  }
}

/**
 * @brief Retrieves a channel by its name.
 * @return Pointer to the Channel object, or NULL if it doesn't exist.
 */
Channel *Server::searchChannel(const std::string &channelName) {
  std::map<std::string, Channel>::iterator it = _channels.find(channelName);
  if (it != _channels.end())
    return &(it->second);
  return NULL;
}

/**
 * @brief Broadcasts a generic message/command to all members of a channel.
 */
void Server::broadcastChannel(Channel *ch, const std::string &command,
                              const std::string &comment, Client &sender) {
  if (ch->clientIsMember(sender.getFd())) {
    std::string msg = ":" + sender.getNickname() + "!" + sender.getUsername() +
                      "@localhost " + command + " " + ch->getChName() + " " +
                      comment + "\r\n";

    std::map<int, std::string> members = ch->getMembers();
    for (std::map<int, std::string>::iterator it = members.begin();
         it != members.end(); ++it) {
      sendMsgToFd(it->first, msg);
    }
    if (DEBUG)
      std::cout << BLUE << "msg : " << msg << " was broadcasted" << ENDCOLOR
                << std::endl;
  } else {
    sendError(ch->getChName(), 442, sender); // ERR_NOTONCHANNEL
  }
}

/**
 * @brief Sends the RPL_NAMREPLY (353) and RPL_ENDOFNAMES (366) to a client.
 * Lists all users in a channel, prefixing operators with '@'.
 */
void Server::sendNames(Channel &ch, Client *client) {
  std::map<int, std::string> members = ch.getMembers();
  std::vector<int> operators = ch.getOperators();
  std::string msg = ":server 353 = " + ch.getChName() + " :";

  for (std::map<int, std::string>::iterator it = members.begin();
       it != members.end(); ++it) {
    bool isOp = false;
    for (size_t j = 0; j < operators.size(); j++) {
      if (it->first == operators[j]) {
        isOp = true;
        break;
      }
    }
    if (isOp)
      msg += "@";

    Client *foundClient = findClient(it->first);
    if (foundClient) {
      msg += foundClient->getNickname() + " ";
    }
  }
  msg += "\r\n";
  sendMsgToClient(msg, *client);

  msg = ":server 366 " + client->getNickname() + " " + ch.getChName() +
        " :End of /NAMES list\r\n";
  sendMsgToClient(msg, *client);
}

/**
 * @brief Handles the INVITE command.
 * Safely parses target nicknames and channel names (e.g., "INVITE user #chan").
 */
void Server::inviteToChan(Client &sender, const std::string &args) {
  size_t spacePos = args.find(' ');
  if (spacePos == std::string::npos) {
    sendError("INVITE", 461, sender); // ERR_NEEDMOREPARAMS
    return;
  }

  std::string targetsStr = args.substr(0, spacePos);
  std::string chanName = args.substr(spacePos + 1);

  Channel *ch = searchChannel(chanName);
  if (!ch) {
    sendError(chanName, 403, sender); // ERR_NOSUCHCHANNEL
    return;
  }

  if (ch->getInviteSettings() && !ch->clientIsOperator(sender.getFd())) {
    sendError(chanName, 482, sender); // ERR_CHANOPRIVSNEEDED
    return;
  }

  std::vector<std::string> targets = split(targetsStr, ',');
  for (std::vector<std::string>::iterator it = targets.begin();
       it != targets.end(); ++it) {
    int targetFD = clientFdsearch(*it);

    if (targetFD == sender.getFd())
      continue; // Don't invite yourself

    if (targetFD > 0) {
      if (!ch->clientIsMember(targetFD)) {
        std::string message = ":" + sender.getNickname() + "!" +
                              sender.getUsername() + "@localhost INVITE " +
                              *it + " " + chanName + "\r\n";
        sendMsgToClient(message, *_clients[targetFD]);
        ch->addInvited(targetFD);
      } else {
        sendError(_clients[targetFD]->getNickname() + " " + chanName, 443,
                  sender); // ERR_USERONCHANNEL
      }
    } else {
      sendError(*it, 401, sender); // ERR_NOSUCHNICK
    }
  }
}

/**
 * @brief Removes a disconnecting client from all channels safely.
 * Deletes any channels that become empty as a result.
 */
void Server::deleteClientFromChannels(int FD, const std::string &reason) {
  std::vector<std::string> channelsToDelete;

  for (std::map<std::string, Channel>::iterator it = _channels.begin();
       it != _channels.end(); ++it) {
    Client *foundClient = findClient(FD);
    if (foundClient) {
      broadcastChannel(&(it->second), "QUIT", reason, *foundClient);
      it->second.deleteClient(FD);
      if (it->second.getMembers().empty()) {
        channelsToDelete.push_back(it->first);
      }
    }
  }

  // Safely erase empty channels AFTER iterating to prevent iterator
  // invalidation
  for (size_t i = 0; i < channelsToDelete.size(); i++) {
    _channels.erase(channelsToDelete[i]);
  }
}

/**
 * @brief Handles the KICK command.
 * Parses channel, targets, and optional reason (e.g., "KICK #chan user
 * :reason").
 */
void Server::kickOutOfChannel(Client &sender, const std::string &args) {
  size_t spacePos = args.find(' ');
  if (spacePos == std::string::npos) {
    sendError("KICK", 461, sender);
    return;
  }

  std::string chanName = args.substr(0, spacePos);
  Channel *ch = searchChannel(chanName);

  if (!ch) {
    sendError(chanName, 403, sender);
    return;
  }

  if (ch->clientIsOperator(sender.getFd())) {
    size_t colonPos = args.find(':');
    std::string reason = "";
    std::string targetsStr;

    if (colonPos != std::string::npos) {
      reason = args.substr(colonPos);

      size_t nextSpace = args.find(' ', spacePos + 1);
      if (nextSpace != std::string::npos && nextSpace < colonPos) {
        targetsStr = args.substr(spacePos + 1, nextSpace - spacePos - 1);
      } else {
        targetsStr = args.substr(spacePos + 1, colonPos - spacePos - 1);
      }
    } else {
      targetsStr = args.substr(spacePos + 1);
    }

    std::vector<std::string> targets = split(targetsStr, ',');
    for (std::vector<std::string>::iterator it = targets.begin();
         it != targets.end(); ++it) {
      int targetFD = clientFdsearch(*it);
      if (ch->clientIsMember(targetFD)) {
        std::string msg = ":" + sender.getNickname() + "!" +
                          sender.getUsername() + "@localhost KICK " + chanName +
                          " " + *it;
        if (!reason.empty())
          msg += " " + reason;
        msg += "\r\n";

        std::map<int, std::string> members = ch->getMembers();
        for (std::map<int, std::string>::iterator memIt = members.begin();
             memIt != members.end(); ++memIt) {
          sendMsgToFd(memIt->first, msg);
        }

        ch->deleteClient(targetFD);
        if (ch->getMembers().empty()) {
          _channels.erase(chanName);
        }
      } else {
        int exist = (targetFD > 0) ? 1 : 0;
        if (exist == 1) {
          sendError(*it + " " + chanName, 441, sender); // ERR_USERNOTINCHANNEL
        } else {
          sendError(*it, 401, sender); // ERR_NOSUCHNICK
        }
      }
    }
  } else {
    if (ch->clientIsMember(sender.getFd()))
      sendError(chanName, 482, sender);
    else
      sendError(chanName, 442, sender);
  }
}

/**
 * @brief Handles the TOPIC command.
 * Updates the channel topic if permissions allow (+t mode check).
 */
void Server::setTopic(Client &sender, const std::string &args) {
  size_t spacePos = args.find(' ');
  size_t colonPos = args.find(':');

  if (spacePos == std::string::npos)
    return;

  std::string chanName = args.substr(0, spacePos);
  Channel *ch = searchChannel(chanName);

  if (!ch) {
    sendError(chanName, 403, sender);
    return;
  }

  if (ch->getTopicRestr() && !ch->clientIsOperator(sender.getFd())) {
    sendError(chanName, 482, sender); // ERR_CHANOPRIVSNEEDED
    return;
  }

  std::string topic =
      (colonPos != std::string::npos) ? args.substr(colonPos + 1) : "";
  ch->setTopic(topic);
  broadcastChannel(ch, "TOPIC", ":" + topic, sender);
}

/**
 * @brief Handles the PART command.
 * Allows a user to gracefully leave one or more channels.
 */
void Server::execPART(Client &sender, const std::string &args) {
  size_t spacePos = args.find(' ');
  size_t colonPos = args.find(':');

  std::string targetsStr = args;
  std::string reason = "";

  if (colonPos != std::string::npos) {
    reason = args.substr(colonPos);
    if (spacePos != std::string::npos && spacePos < colonPos) {
      targetsStr = args.substr(0, spacePos);
    } else {
      targetsStr = args.substr(0, colonPos - 1);
    }
  } else if (spacePos != std::string::npos) {
    targetsStr = args.substr(0, spacePos);
  }

  std::vector<std::string> targets = split(targetsStr, ',');

  for (std::vector<std::string>::iterator it = targets.begin();
       it != targets.end(); ++it) {
    Channel *ch = searchChannel(*it);
    if (!ch) {
      sendError(*it, 403, sender);
    } else {
      broadcastChannel(ch, "PART", reason, sender);
      ch->deleteClient(sender.getFd());
      if (ch->getMembers().empty()) {
        _channels.erase(*it);
      }
    }
  }
}

/**
 * @brief Utility function to send a raw string message to a specific file
 * descriptor.
 */
void Server::sendMsgToFd(int fd, const std::string &msg) {
  Client *foundClient = findClient(fd);
  if (foundClient) {
    sendMsgToClient(msg, *foundClient);
  }
}
