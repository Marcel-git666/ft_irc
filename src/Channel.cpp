#include "Channel.hpp"

Channel::Channel() : _members(), _operators(), _invited(), _name(""), _topic(""), _modes(""), _key("") {
	_invite_only = false;
	_topic_restrictions = false;
	_key_settings = false;
	_user_limit =  -1;
}

Channel::Channel(Client *client, std::string name) : _invited(), _name(name), _topic(""), _modes(""), _key("") {
	addMember(client);
	addOperator(client);
	_invite_only = false;
	_topic_restrictions = false;
	_key_settings = false;
	_user_limit =  -1;
}

Channel::Channel(const Channel &other) {
	_members = other._members;
	_operators = other._operators;
	_name = other._name;
	_topic = other._topic;
	_modes = other._modes;
	_key = other._key;
	_invite_only = other._invite_only;
	_topic_restrictions = other._topic_restrictions;
	_key_settings = other._key_settings;
	_user_limit =  other._user_limit;
}

Channel& Channel::operator=(const Channel &other) {
	if (this != &other) {
		_members = other._members;
		_operators = other._operators;
		_name = other._name;
		_topic = other._topic;
		_modes = other._modes;
		_key = other._key;
		_invite_only = other._invite_only;
		_topic_restrictions = other._topic_restrictions;
		_key_settings = other._key_settings;
		_user_limit =  other._user_limit;
	}
	return (*this);
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

bool Channel::clientIsMember(std::string clientNickname) {
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++) {
		if ((*it)->getNickname() == clientNickname)
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

std::string Channel::getModestring() {
	return (this->_modestring);
}

bool Channel::getKeySetting() {
	return(this->_key_settings);
}

bool  Channel::getTopicRestr() {
	return(this->_topic_restrictions);
}

bool  Channel::getInviteSettings() {
	return(this->_invite_only);
}

int  Channel::getUserLimit() {
	return(this->_user_limit);
}


void Channel::deleteClient(int FD) {
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++) {
		if ((*it)->getFd() == FD) {
			_members.erase(it);
			break ;
		}
	}
	for (std::vector<Client*>::iterator itOp = _operators.begin(); itOp != _operators.end(); itOp++) {
		if ((*itOp)->getFd() == FD) {
			_operators.erase(itOp);
			break ;
		}
	}
}

void Channel::deleteClient(std::string clientNickname) {
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++) {
		if ((*it)->getNickname() == clientNickname) {
			_members.erase(it);
			break ;
		}
	}
	for (std::vector<Client*>::iterator itOp = _operators.begin(); itOp != _operators.end(); itOp++) {
		if ((*itOp)->getNickname() == clientNickname) {
			_operators.erase(itOp);
			break ;
		}
	}
}

bool Channel::addMode(char mode) {
	if (mode == 'i' || mode == 'k' || mode == 't' || mode == 'l') {
		this->_modes += mode;
		return (true);
	}
	return (false);
}

bool Channel::delMode(char mode) {
	size_t pos = this->_modes.find(mode);
	if (pos != std::string::npos) {
		_modes.erase(pos);
		return (true);
	}
	return (false);
}