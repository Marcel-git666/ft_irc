#include "Server.hpp"

void Server::sendChanMode(Client& sender, Channel* chan) {
	std::cout << BLUE << "sending mode of chan " << chan->getChName() 
		<< " to the client " << sender.getNickname() << ENDCOLOR << std::endl;
	std::string msg = ":server 324 " + chan->getChName() + " +";
	if (!chan->getModestring().empty()) {
		msg += chan->getModestring();
		for (size_t i = 0; i < chan->getModestring().length(); i++) {
			if (chan->getModestring()[i] == 'k')
				msg += " " + chan->getKey();
			if (chan->getModestring()[i] == 'l')
				msg += " " + chan->getLimitString();
		}
	}
	sendMsgToClient(msg + "\r\n", sender);
}

void Server::applyMode(Client& sender, Channel* chan, std::string modestring) {
	if (chan->clientIsOperator(&sender)) {
		size_t spasePos = modestring.find(" ");
		std::vector<std::string> modeARGs;
		int modeRes;
		if (spasePos != std::string::npos) {
			modeARGs = split(modestring.substr(spasePos + 1), ' ');
			modestring = modestring.substr(0, spasePos);
		}
		if (modestring[0] == '+') {
			if (modestring.find("k") != std::string::npos && modestring.find("l") != std::string::npos && modeARGs.size() < 2) {
				sendError("MODE", 461, sender);
				return ;
			}
			for (size_t i = 1; i < modestring.length(); i++) {
				if (chan->getModestring().find(modestring[i]) == std::string::npos || modestring[i] == 'l' || modestring[i] == 'k') {
				//Ira: chaeck if mode isn't in a string, for k and l its possible, if client want to change args
					modeRes = chan->addMode(modestring[i], modeARGs);
					switch (modeRes)
					{
						case (461):
							sendError("MODE", 461, sender);
							return ;
						case (472):
							std::string char_to_str(1, modestring[i]);
							sendError(char_to_str, 472, sender);
							return ;
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
	else {
		if (chan->clientIsMember(&sender))
			sendError(chan->getChName(), 482, sender);
		else
			sendError(chan->getChName(), 442, sender);
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
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ) {
		Channel *ch = searchChannel(*it);
		if (!ch) { //check if channel exists
			sendError(*it, 403, sender);
			it = targets.erase(it); // Ira: erase returns next valid iterator
		}
		else {
			if (modestring != "") {
				applyMode(sender, ch, modestring);
				std::vector<Client*> members = ch->getMembers();
				for (size_t i = 0; i < members.size(); i++) {
						sendNames(*ch, members[i]);
				}
			}
			else
				sendChanMode(sender, ch);
			++it;
		}
	}
}
