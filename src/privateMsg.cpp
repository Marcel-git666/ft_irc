#include "../inc/Server.hpp"

/**
 * @brief Splits a string by a given delimiter.
 * Useful for extracting multiple targets (e.g., "bob,alice,judy" -> ["bob",
 * "alice", "judy"]).
 * * @param targetsStr The string containing separated values.
 * @param delimeter The character used to split the string (usually ',').
 * @return std::vector<std::string> A vector of extracted strings.
 */
std::vector<std::string> Server::split(const std::string &targetsStr,
                                       char delimeter) {
  std::vector<std::string> targets;
  size_t start = 0;
  size_t pos = targetsStr.find(delimeter);

  while (pos != std::string::npos) {
    targets.push_back(targetsStr.substr(start, pos - start));
    start = pos + 1;
    pos = targetsStr.find(delimeter, start);
  }
  // Add the last target after the final delimiter
  targets.push_back(targetsStr.substr(start));
  return targets;
}

/**
 * @brief Handles the PRIVMSG command for sending messages to individual
 * clients. Supports sending the same message to multiple clients
 * simultaneously.
 * * @param sender The client sending the message.
 * @param args The raw arguments after the PRIVMSG command (e.g., "bob,judy
 * :Hello!").
 */
void Server::sendPrivateMsg(const Client &sender, const std::string &args) {
  size_t spacePos = args.find(' ');
  size_t colonPos = args.find(':');

  // Validate the IRC message format (must have targets and a message body)
  if (spacePos == std::string::npos || colonPos == std::string::npos) {
    sendError("PRIVMSG", 461, sender); // ERR_NEEDMOREPARAMS
    return;
  }

  // Safely extract targets (everything up to the first space) and the message
  // body
  std::string targetsStr = args.substr(0, spacePos);
  std::string msg = args.substr(colonPos); // keeps the colon, e.g., ":Hello!"

  std::vector<std::string> targets = split(targetsStr, ',');

  // Iterate over all targets and send the message
  for (std::vector<std::string>::iterator it = targets.begin();
       it != targets.end(); ++it) {
    int targetFD = clientFdsearch(*it);

    if (targetFD > 0) {
      std::string message = ":" + sender.getNickname() + "!" +
                            sender.getUsername() + "@localhost PRIVMSG " + *it +
                            " " + msg + "\r\n";

      if (DEBUG) {
        std::cout << GREEN << "Sending message from " << sender.getNickname()
                  << " to " << _clients[targetFD]->getNickname() << ENDCOLOR
                  << std::endl;
      }
      sendMsgToClient(message, *_clients[targetFD]);
    } else {
      // User not found
      sendError(*it, 401, sender); // ERR_NOSUCHNICK
    }
  }
}

/**
 * @brief Handles the PRIVMSG command for sending messages to a whole channel.
 * Broadcasts the message to all channel members except the sender.
 * * @param sender The client sending the message.
 * @param args The raw arguments after the PRIVMSG command (e.g., "#general :Hi
 * all!").
 */
void Server::sendToChannel(Client &sender, const std::string &args) {
  size_t spacePos = args.find(' ');
  size_t colonPos = args.find(':');

  if (spacePos == std::string::npos || colonPos == std::string::npos) {
    sendError("PRIVMSG", 461, sender); // ERR_NEEDMOREPARAMS
    return;
  }

  std::string channelName = args.substr(0, spacePos);
  std::string msgBody = args.substr(colonPos); // keeps the colon

  Channel *ch = searchChannel(channelName);

  if (!ch) {
    sendError(channelName, 403, sender); // ERR_NOSUCHCHANNEL
    return;
  }

  // Check if the sender is actually a member of the channel
  if (ch->clientIsMember(sender.getFd())) {
    std::string fullMessage = ":" + sender.getNickname() + "!" +
                              sender.getUsername() + "@localhost PRIVMSG " +
                              channelName + " " + msgBody + "\r\n";

    std::map<int, std::string> members = ch->getMembers();
    for (std::map<int, std::string>::iterator it = members.begin();
         it != members.end(); ++it) {
      // Send to everyone EXCEPT the person who wrote it
      if (sender.getFd() != it->first) {
        sendMsgToFd(it->first, fullMessage);
      }
    }
  } else {
    // Cannot send messages to channels you haven't joined
    sendError(channelName, 442, sender); // ERR_NOTONCHANNEL
  }
}
