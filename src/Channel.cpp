#include "Channel.hpp"

Channel::Channel() : _name(""), _topic("") {}

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