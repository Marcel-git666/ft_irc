#include "../inc/Server.hpp"

void Server::connectToChannel(Client* client, std::string& name, std::string key) {
	Channel *ch = searchChannel(name);
	if (!ch) {
		std::pair<std::map<std::string, Channel>::iterator, bool> result;
		result = _channels.insert(std::make_pair(name, Channel(client, name)));
		ch = &result.first->second; // safe pointer
	}
	else {
		if (ch->getKeySetting() && key != ch->getKey()) {
			sendError(ch->getChName(), 475, *client);
			return ;
		}
		if (ch->getInviteSettings() && !ch->userInvited(client->getFd())) {
			sendError(ch->getChName(), 473, *client);
			return ;
		}
		if (ch->isFull()) {
			sendError(ch->getChName(), 471, *client);
			return ;
		}
		ch->addMember(client->getFd(), client->getNickname());
		if (ch->getInviteSettings())
			ch->removeFromInvited(client->getFd());
	}
	std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost JOIN " + name + "\r\n";
	std::map<int, std::string> members = ch->getMembers();
	for (std::map<int, std::string>::iterator it = members.begin(); it != members.end(); it++) //Ira: we need to send info about member joined to all of the channel members
		sendMsgToClient(msg, *(findClient(it->first)));
	std::string topic = ch->getTopic();
	if (topic != "")
		msg = ":server 332 " + client->getNickname() + " " + name + " :" + topic + "\r\n";
	else
		msg = ":server 331 " + client->getNickname() + " " + name + " :No topic is set\r\n";
	sendMsgToClient(msg, *client);
	sendNames(*ch, client);
}

void Server::joinChannel(Client* client, std::string& args) {
	std::vector<std::string> targets;
	std::string key = "";
	size_t commaPos = args.find(","); //Ira: client send a list of channels he wants to join separated by commas
	size_t spacePos = args.find(" ");
	if (spacePos != std::string::npos) {
		key = args.substr(spacePos + 1);
		args.erase(spacePos);
	}
	if (commaPos != std::string::npos) {
		targets = split(args, ',');
	}
	else
		targets.push_back(args);
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); it++) {
		connectToChannel(client, *it, key);
	}
}

Channel* Server::searchChannel(const std::string& channelName) {
	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	if (it != _channels.end())
		return &(it->second);
	return NULL;
}

void Server::sendToChannel(Client &sender, std::string args) {
	size_t colonPos = args.find(":");
	std::string channelName = args.substr(0, colonPos - 1);
	Channel *ch = searchChannel(channelName);
	if (!ch)
		sendError(args, 403, sender);
	else {
		if (ch->clientIsMember(sender.getFd())) {
			std::string msg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@localhost PRIVMSG " + args + "\r\n";
			std::map<int, std::string> members = ch->getMembers();
			for (std::map<int, std::string>::iterator it = members.begin(); it != members.end(); it++)  {
				if (sender.getFd() != it->first)
					sendMsgToClient(msg,  *(findClient(it->first)));
			}
		}
		else
			sendError(channelName, 442, sender);
	}
}

void Server::broadcastChannel(Channel* ch, std::string command, std::string comment, Client& sender) {
	if (ch->clientIsMember(sender.getFd())) {
			std::string msg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@localhost " + command + " " + ch->getChName() + " " + comment + "\r\n";
			std::map<int, std::string> members = ch->getMembers();
			for (std::map<int, std::string>::iterator it = members.begin(); it != members.end(); it++) {
					sendMsgToClient(msg, *(findClient(it->first)));
			}
			if (DEBUG)
				std::cout << BLUE << "msg : " << msg << " was broadcasted" << ENDCOLOR << std::endl;
		}
	else
		sendError(ch->getChName(), 442, sender);
}

void Server::sendNames(Channel& ch, Client *client) {
	std::map<int, std::string> members = ch.getMembers();
	std::vector<int> operators = ch.getOperators();
	std::string msg = ":server 353 = " + ch.getChName() + " :";
	for (std::map<int, std::string>::iterator it = members.begin(); it != members.end(); it++) {
		//if (!it) continue;
		bool isOp = false;
		for (size_t j = 0; j < operators.size(); j++) {
			if (it->first == operators[j]) {
				isOp = true;
				break;
			}
		}
		if (isOp)
			msg += "@";
		msg += findClient(it->first)->getNickname();
		msg += " ";
	}
	msg += "\r\n";
	sendMsgToClient(msg, *client);
	msg = ":server 366 " + client->getNickname() + " " + ch.getChName() + " :End of /NAMES list\r\n";
	sendMsgToClient(msg, *client);
}

void Server::inviteToChan(Client& sender, std::string args) {
	std::vector<std::string> targets;
	size_t hashtagPos = args.find("#");
	std::string chanName = args.substr(hashtagPos);
	Channel *ch = searchChannel(chanName);
	if (!ch) { //check if channel exists
		sendError(chanName, 403, sender);
		return ;
	}
	if (ch->getInviteSettings() && !ch->clientIsOperator(sender.getFd())) {
		sendError(chanName, 482, sender);
		return ;
	}
	if (hashtagPos != std::string::npos) {
		std::string targetsStr = args.substr(0, hashtagPos - 1); //Ira: need to cut space and hashtag
		targets = split(targetsStr, ',');
		std::string msg = args.substr(hashtagPos);
		for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); it++) {
			int FD = clientFdsearch(*it); //Ira: need to find if client exists
			if (FD == sender.getFd()) //Ira: avoiding to send invitation to himself
				continue ;
			if(FD > 0) {
				if (!ch->clientIsMember(FD)) { //Ira: if client isn't a member of channel
					std::string message = ":" + sender.getNickname() + "!" +
						sender.getUsername() + "@localhost INVITE " +
						*it + " " + msg + "\r\n";
					if (DEBUG)
						std::cout << GREEN << "Sending message " << msg << " from " << sender.getNickname() << " to " << _clients[FD]->getNickname() << ENDCOLOR << std::endl;
					sendMsgToClient(message, *_clients[FD]); 
					ch->addInvited(FD);
				}
				else //Ira:client already on channel
					sendError(_clients[FD]->getNickname() + " " + chanName, 443, sender);
			}
			else
				sendError(*it, 401, sender);
		}
	}
	else {
		sendError(args, 403, sender);
	}
}

void Server::deleteClientFromChannels(int FD, std::string reason) {
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++) {
		broadcastChannel(&(it->second), "QUIT", reason, *(findClient(FD)));
		it->second.deleteClient(FD);
	}
}

void Server::kickOutOfChannel(Client &sender, std::string args) {
	std::vector<std::string> targets;
	size_t spasePos = args.find(" ");
	std::string reason = "";
	std::string chanName = args.substr(0, spasePos);
	Channel *ch = searchChannel(chanName);
	if (!ch) { //check if channel exists
		sendError(chanName, 403, sender);
		return ;
	}
	else {
		if (ch->clientIsOperator(sender.getFd())) {
			size_t colonPos = args.find(":");
			if (colonPos != std::string::npos)
				reason = args.substr(colonPos);
			else
				colonPos = args.length();
			std::string targetsStr = args.substr(spasePos + 1, colonPos - (spasePos + 1));
			if (DEBUG)
				std::cout << "Target string " << targetsStr << std::endl;
			targets = split(targetsStr, ',');
			for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); it++) {
				if (ch->clientIsMember(clientFdsearch(*it))) {
					std::string msg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@localhost KICK " + chanName + " " + *it;
					if (!reason.empty())
						msg += " " + reason;
					msg += "\r\n";
					std::map<int, std::string> members(ch->getMembers());
					for (std::map<int, std::string>::iterator it = members.begin(); it != members.end(); it++) //Ira: we need to send info about member kicked of the channel members
						sendMsgToClient(msg, *(findClient(it->first)));
					ch->deleteClient(clientFdsearch(*it));
				}
				else {
					int exist = 0;
					for (std::map<int, Client *>::iterator it_cli =_clients.begin(); it_cli != _clients.end(); it_cli++) {
						if (it_cli->second->getNickname() == *it) {
							exist = 1;
							sendError(*it + " " + chanName, 441, sender);
							break;
						}
					}
					if (exist == 0)
						sendError(*it, 401, sender);
				}
			}
		}
		else {
			if (ch->clientIsMember(sender.getFd()))
				sendError(chanName, 482, sender);
			else
				sendError(chanName, 442, sender);
		}
	}
}

void Server::setTopic(Client& sender, std::string& args) {
	size_t spacePos = args.find(" ");
	size_t colonPos = args.find(":");
	std::string topic = args.substr(colonPos + 1);
	std::string chanName = args.erase(spacePos);
	Channel *ch = searchChannel(chanName);
	if (!ch) { //check if channel exists
		sendError(chanName, 403, sender);
		return ;
	}
	else {
		if (ch->getTopicRestr() && !ch->clientIsOperator(sender.getFd())) {
			sendError(chanName, 482, sender);
			return ;
		}
		ch->setTopic(topic);
		broadcastChannel(ch, "TOPIC", ":" + topic, sender);
	}
}

void Server::execPART(Client& sender, std::string& args) {
	std::vector<std::string> targets;
	std::string targetsStr;
	size_t spasePos = args.find(" ");
	std::string reason = "";
	size_t colonPos = args.find(":");
	if (colonPos != std::string::npos)
		reason = args.substr(colonPos);
	else
		colonPos = args.length();
	if (spasePos != std::string::npos) {
		targetsStr = args.substr(0, spasePos);
		targets = split(targetsStr, ',');
	}
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); it++) {
		Channel* ch;
		ch = searchChannel(*it);
		if (!ch)
			sendError(*it, 403, sender);
		else {
			broadcastChannel(ch, "PART", reason, sender);
			ch->deleteOperator(sender.getFd());
			ch->deleteClient(sender.getFd());
		}
	}
}