#include "Server.hpp"

void Server::connectToChannel(Client* client, std::string& name) {
	Channel *ch = searchChannel(name);
	if (!ch)
		_channels.insert(std::map<std::string, Channel>::value_type(name, Channel(client, name)));
	else
		ch->addMember(client);
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
			std::string msg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@localhost PRIVMSG " + args;
			std::vector<Client*> members = ch->getMembers();
			for (size_t i = 0; i < members.size(); i++)
				sendMsgToClient(msg, *members[i]);
		}
	}
}


// void Server::kickOutOfChannel(Client &client, std::string args) {
	
// }