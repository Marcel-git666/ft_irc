#include "Server.hpp"

void Server::sendChanMode(Client& sender, Channel* chan) {
	//Need to send 324 <nick> #channel +<modes> [parameters]
	std::cout << "sending mode of chan " << chan->getChName() 
		<< " to the client " << sender.getNickname() << std::endl;
}

void Server::applyMode(Client& sender, Channel* chan, std::string modestring) {
	if (modestring[0] == '+') {
		for (size_t i = 1; i < modestring.length(); i++) {
			if (chan->getModestring().find(modestring[i]) == std::string::npos) {
				if (!chan->addMode(modestring[i])) {
					std::string char_to_str(1, modestring[i]);
					sendError(char_to_str, 472, sender);
				}
			}
		}
	}
	else if (modestring[0] == '-') {
		for (size_t i = 1; i < modestring.length(); i++) {
			chan->delMode(modestring[i]);
		}
	}
}

void Server::operateMode(Client& sender, std::string args) {
	size_t spacePos = args.find(" ");
	size_t commaPos = args.find(",");
	std::string targetStr = args;
	std::string modestring = "";
	std::vector<std::string> targets;
	if (spacePos != std::string::npos) {
		modestring = args.substr(spacePos + 1);
		targetStr = args.substr(0, spacePos);
	}
	if (commaPos != std::string::npos) {
		targets = split(targetStr, ',');
	}
	else
		targets.push_back(targetStr);
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); it++) {
		Channel *ch = searchChannel(*it);
		if (!ch) { //check if channel exists
			sendError(*it, 403, sender);
			targets.erase(it);
			
		}
		else {
			if (modestring != "")
				applyMode(sender, ch, modestring);
			else
				sendChanMode(sender, ch);
		}
	}
}
