#include "Channel.hpp"

Channel::Channel() : _members(), _operators(), _name(""), _topic("") {}

Channel::Channel(Client *client, std::string name) : _name(name), _topic("") {
	addMember(client);
	addOperator(client);
}

Channel& Channel::operator=(const Channel &other) {
	if (this != &other) {
		_members = other._members;
		_operators = other._operators;
		_name = other._name;
		_topic = other._topic;
	}
	return (*this);
}

Channel::Channel(const Channel &other) {
	_members = other._members;
	_operators = other._operators;
	_name = other._name;
	_topic = other._topic;
}

Channel::~Channel() {}

void Channel::addMember(Client *newMember) {
	_members.push_back(newMember);
}

void Channel::addOperator(Client *newOper) {
	_operators.push_back(newOper);
}

bool Channel::clientIsOperator(Client *client) {
	for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); it++) {
		if (*it == client)
			return (true);
	}
	return (false);
}

bool Channel::clientIsMember(Client *client) {
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++) {
		if (*it == client)
			return (true);
	}
	return (false);
}

bool Channel::clientIsMember(int FD) {
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++) {
		if ((*it)->getFd() == FD)
			return (true);
	}
	return (false);
}

std::vector<Client *> Channel::getMembers() {
	return (_members);
}

std::vector<Client *> Channel::getOperators() {
	return (_operators);
}

Client* Channel::getClientFromChan(int FD) {
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++) {
		if ((*it)->getFd() == FD)
			return (*it);
	}
	return (NULL);
}

std::string Channel::getTopic() {
	return (_topic);
}

std::string Channel::getChName() {
	return (_name);
}