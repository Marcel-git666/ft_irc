#include "Server.hpp"

void Server::connectToChannel(Client* client, std::string& name) {
	Channel *ch = searchChannel(name);
	if (!ch) {
		std::pair<std::map<std::string, Channel>::iterator, bool> result;
		result = _channels.insert(std::make_pair(name, Channel(client, name)));
		ch = &result.first->second; // safe pointer
	}
	else {
		ch->addMember(client);
	}
	std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost JOIN " + name + "\r\n";
	std::vector<Client*> members = ch->getMembers();
	for (size_t i = 0; i < members.size(); i++) //Ira: we need to send info about member joined to all of the channel members
		sendMsgToClient(msg, *members[i]);
	std::string topic = ch->getTopic();
	if (topic != "")
		msg = ":server 332 " + client->getNickname() + " " + name + ":" + topic + "\r\n";
	else
		msg = ":server 331 " + client->getNickname() + " " + name + ":No topic is set\r\n";
	sendNames(*ch, client);
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
		if (ch->clientIsMember(&sender)) {
			std::string msg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@localhost PRIVMSG " + args + "\r\n";
			std::vector<Client*> members = ch->getMembers();
			for (size_t i = 0; i < members.size(); i++) {
				if (&sender != members[i])
					sendMsgToClient(msg, *members[i]);
			}
		}
	}
}

void Server::sendNames(Channel& ch, Client *client) {
	std::vector<Client*> members = ch.getMembers();
	std::vector<Client*> operators = ch.getOperators();
	std::string msg = ":server 353 = " + ch.getChName() + " :";
	for (size_t i = 0; i < members.size(); i++) {
		if (!members[i]) continue;
		bool isOp = false;
		for (size_t j = 0; j < operators.size(); j++) {
			if (members[i] == operators[j]) {
				isOp = true;
				break;
			}
		}
		if (isOp)
			msg += "@";
		msg += members[i]->getNickname();
		msg += " ";
	}
	msg += "\r\n";
	sendMsgToClient(msg, *client);
	msg = ":server 366 " + ch.getChName() + ":End of /NAMES list\r\n";
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
	if (hashtagPos != std::string::npos) {
		std::string targetsStr = args.substr(0, hashtagPos - 1); //Ira: need to cut space and hashtag
		targets = split(targetsStr, ',');
		std::string msg = args.substr(hashtagPos);
		for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); it++) {
			int FD = clientFdsearch(*it);
			if (FD == sender.getFd())
				continue;
			if(FD > 0) {
				if (!ch->clientIsMember(FD)) {
					std::string message = ":" + sender.getNickname() + "!" +
						sender.getUsername() + "@localhost INVITE " +
						*it + " " + msg + "\r\n";
					std::cout << GREEN << "Sending message from " << sender.getNickname() << " to " << _clients[FD]->getNickname() << ENDCOLOR << std::endl;
					sendMsgToClient(message, *_clients[FD]);
				}
				else
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
// void Server::kickOutOfChannel(Client &client, std::string args) {
	
// }