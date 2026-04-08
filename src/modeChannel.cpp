#include "../inc/Server.hpp"

/**
 * @brief Sends the current modes of a channel to a client.
 * Triggered when a user types `/mode #channel` without any arguments.
 * Responses use the standard RPL_CHANNELMODEIS (324) format.
 * * @param sender The client requesting the modes.
 * @param chan Pointer to the requested channel.
 */
void Server::sendChanMode(Client &sender, Channel *chan) {
  if (!chan->clientIsMember(sender.getFd())) {
    sendError(chan->getChName(), 442, sender); // ERR_NOTONCHANNEL
    return;
  }

  if (DEBUG) {
    std::cout << BLUE << "Sending modes of channel " << chan->getChName()
              << " to client " << sender.getNickname() << ENDCOLOR << std::endl;
  }

  std::string msg = ":server 324 " + chan->getChName() + " +";

  // Append all active mode characters (e.g., "itk")
  if (!chan->getModestring().empty()) {
    msg += chan->getModestring();

    // If the channel has a key (+k) or user limit (+l), append their values
    for (size_t i = 0; i < chan->getModestring().length(); i++) {
      if (chan->getModestring()[i] == 'k') {
        msg += " " + chan->getKey();
      }
      if (chan->getModestring()[i] == 'l') {
        msg += " " + chan->getLimitString();
      }
    }
  }
  sendMsgToClient(msg + "\r\n", sender);
}

/**
 * @brief Parses and applies mode changes to a channel (+/- i, k, l, t, o).
 * Validates that the sender has operator privileges before applying changes.
 * * @param sender The client attempting to change modes.
 * @param chan Pointer to the target channel.
 * @param modestring The string containing mode flags and arguments (e.g., "+k
 * secretpass").
 */
void Server::applyMode(Client &sender, Channel *chan,
                       const std::string &modestring) {
  // Only operators can change channel modes
  if (!chan->clientIsOperator(sender.getFd())) {
    if (chan->clientIsMember(sender.getFd())) {
      sendError(chan->getChName(), 482, sender); // ERR_CHANOPRIVSNEEDED
    } else {
      sendError(chan->getChName(), 442, sender); // ERR_NOTONCHANNEL
    }
    return;
  }

  size_t spacePos = modestring.find(' ');
  std::string modes;
  std::vector<std::string> modeARGs;
  int modeRes;

  // Split modestring into the mode flags (e.g., "+ol") and their arguments
  // (e.g., "user 50")
  if (spacePos != std::string::npos) {
    modes = modestring.substr(0, spacePos);
    modeARGs = split(modestring.substr(spacePos + 1), ' ');
    if (DEBUG) {
      std::cout << BLUE << "Modestring applied: " << modestring << ENDCOLOR
                << std::endl;
    }
  } else {
    modes = modestring;
  }

  // --- ADDING MODES (+) ---
  if (modes[0] == '+') {
    // Special check: +k and +l both require arguments. If both are passed but
    // there's < 2 args, fail.
    if (modes.find('k') != std::string::npos &&
        modes.find('l') != std::string::npos && modeARGs.size() < 2) {
      sendError("MODE", 461, sender); // ERR_NEEDMOREPARAMS
      return;
    }

    for (size_t i = 1; i < modes.length(); i++) {
      // Apply mode only if it isn't already active, EXCEPT for 'l' and 'k'
      // which can be overwritten
      if (chan->getModestring().find(modes[i]) == std::string::npos ||
          modes[i] == 'l' || modes[i] == 'k') {
        modeRes = chan->addMode(modes[i], modeARGs);
        switch (modeRes) {
        case 441: // ERR_USERNOTINCHANNEL (for +o)
          sendError(modeARGs[0] + " " + chan->getChName(), 441, sender);
          return;
        case 461: // ERR_NEEDMOREPARAMS
          sendError("MODE", 461, sender);
          return;
        case 472: { // ERR_UNKNOWNMODE
          std::string char_to_str(1, modes[i]);
          sendError(char_to_str, 472, sender);
          return;
        }
        }
      }
    }
  }
  // --- REMOVING MODES (-) ---
  else if (modes[0] == '-') {
    for (size_t i = 1; i < modes.length(); i++) {
      modeRes = chan->delMode(modes[i], modeARGs);
      switch (modeRes) {
      case 441: // ERR_USERNOTINCHANNEL (for -o)
        sendError(modeARGs[0] + " " + chan->getChName(), 441, sender);
        return;
      case 461: // ERR_NEEDMOREPARAMS
        sendError("MODE", 461, sender);
        return;
      case 472: { // ERR_UNKNOWNMODE
        std::string char_to_str(1, modes[i]);
        sendError(char_to_str, 472, sender);
        return;
      }
      }
      // If a target user was provided for -o, check if they exist on the server
      if (!modeARGs.empty() && clientFdsearch(modeARGs[0]) == -1) {
        sendError(modeARGs[0], 401, sender); // ERR_NOSUCHNICK
        return;
      }
    }
  }

  // Successfully updated modes, broadcast the change to the channel
  broadcastChannel(chan, "MODE", modestring, sender);
}

/**
 * @brief Entry point for the MODE command.
 * Parses the target channels and decides whether to send mode info or apply
 * mode changes.
 * * @param sender The client sending the MODE command.
 * @param args The arguments of the MODE command (e.g., "#chan1,#chan2 +k pass"
 * or just "#chan1").
 */
void Server::operateMode(Client &sender, const std::string &args) {
  size_t spacePos = args.find(' ');
  std::string targetStr = args;
  std::string modestring = "";
  std::vector<std::string> targets;

  // Separate the target channels from the actual mode instructions
  if (spacePos != std::string::npos) {
    targetStr = args.substr(0, spacePos);
    modestring = args.substr(spacePos + 1);
  }

  size_t commaPos = targetStr.find(',');
  if (commaPos != std::string::npos) {
    targets = split(targetStr, ',');
  } else {
    targets.push_back(targetStr);
  }

  // Process the command for each targeted channel
  for (std::vector<std::string>::iterator it = targets.begin();
       it != targets.end();) {
    Channel *ch = searchChannel(*it);

    if (!ch) {
      sendError(*it, 403, sender); // ERR_NOSUCHCHANNEL
      it = targets.erase(it);      // Erase invalid targets and continue safely
    } else {
      if (!modestring.empty()) {
        applyMode(sender, ch, modestring); // Attempt to change modes
      } else {
        sendChanMode(sender, ch); // Just request info
      }
      ++it;
    }
  }
}
