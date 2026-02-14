#pragma once

#include "Server.hpp"

void Server::createNewChannel(Client* client, std::string& name) {
	_channels.insert(std::map<std::string, Channel>::value_type(name, Channel(client, name)));
}

void Server::connectToChannel(Client* client, std::string& name) {
	if (!searchChannel(name))
		createNewChannel(client, name);
	else {
		Channel *ch = searchChannel(name);
		ch->addMember(client);
	}
}

Channel* Server::searchChannel(const std::string& channelName) {
	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	if (it != _channels.end())
		return &(it->second);
	return NULL;
}