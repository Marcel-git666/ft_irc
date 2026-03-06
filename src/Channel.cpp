#include "Channel.hpp"

Channel::Channel() : _members(), _operators(), _invited_FD(), _name(""), _topic(""), _modes(""), _key(""), _limit_string("") {
	_invite_only = false;
	_topic_restricted = false;
	_key_settings = false;
	_limit_numeric =  -1;
}

Channel::Channel(Client *client, std::string name) : _invited_FD(), _name(name), _topic(""), _modes(""), _key(""), _limit_string("") {
	addMember(client);
	addOperator(client);
	_invite_only = false;
	_topic_restricted = false;
	_key_settings = false;
	_has_limit = false;
	_limit_numeric =  -1;
}

Channel::Channel(const Channel &other) {
	_members = other._members;
	_operators = other._operators;
	_invited_FD = other._invited_FD;
	_name = other._name;
	_topic = other._topic;
	_modes = other._modes;
	_key = other._key;
	_invite_only = other._invite_only;
	_topic_restricted = other._topic_restricted;
	_key_settings = other._key_settings;
	_has_limit = other._has_limit;
	_limit_numeric =  other._limit_numeric;
	_limit_string = other._limit_string;
}

Channel& Channel::operator=(const Channel &other) {
	if (this != &other) {
		_members = other._members;
		_operators = other._operators;
		_invited_FD = other._invited_FD;
		_name = other._name;
		_topic = other._topic;
		_modes = other._modes;
		_key = other._key;
		_invite_only = other._invite_only;
		_topic_restricted = other._topic_restricted;
		_key_settings = other._key_settings;
		_limit_numeric =  other._limit_numeric;
		_limit_string = other._limit_string;
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

int Channel::deleteOperator(Client *Oper) {
	std::vector<Client*>::iterator itOp;
	for (itOp = _operators.begin(); itOp != _operators.end(); itOp++) {
		if ((*itOp)->getFd() == Oper->getFd()) {
			_operators.erase(itOp);
			break ;
		}
	}
	if (itOp == _operators.end())
		return (-1);
	return (0);
}

void Channel::addInvited(int FD_inv) {
	_invited_FD.push_back(FD_inv);
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
	return (this->_modes);
}

std::string Channel::getKey() {
	return (this->_key);
}

std::string Channel::getLimitString() {
	return (this->_limit_string);
}

int  Channel::getLimitNumeric() {
	return (this->_limit_numeric);
}

bool Channel::getKeySetting() {
	return (this->_key_settings);
}

bool  Channel::getTopicRestr() {
	return (this->_topic_restricted);
}

bool  Channel::getInviteSettings() {
	return (this->_invite_only);
}

bool Channel::isFull() {
	if ((int)_members.size() == _limit_numeric)
		return (true);
	return (false);
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
	for (std::vector<int>::iterator itInv = _invited_FD.begin(); itInv != _invited_FD.end(); itInv++) {
		if (*itInv == FD) {
			_invited_FD.erase(itInv);
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

int Channel::addMode(char mode, std::vector<std::string>& modeARGs) {
	switch (mode) {
		case ('i'):
			this->_modes += mode;
			this->_invite_only = true;
			return (0);
		case ('k'):
			if (modeARGs.empty()) //Ira: if no argument from user
				return (461);
			if (_modes.find(mode) == std::string::npos) //Ira: if we have this mode for channel we don't need to add it to the string, we just need to change the arg
				this->_modes += mode;
			this->_key_settings = true;
			this->_key = modeARGs[0];
			modeARGs.erase(modeARGs.begin());
			return (0);
		case ('t'):
			this->_modes += mode;
			this->_topic_restricted = true;
			return (0);
		case ('l'): {
			if (modeARGs.empty()) //Ira: if no argument from user
				return (461);
			std::stringstream ss(modeARGs[0]);
			if (!(ss >> this->_limit_numeric) || _limit_numeric < 0) 
				return (461); //Ira: IRC does't have any other error if args is incorrect
			_limit_string = modeARGs[0]; //Ira: if we have proper arg for int then we can wright down to string
			if (_modes.find(mode) == std::string::npos) //Ira: if we have this mode for channel we don't need to add it to the string, we just need to change the arg
				this->_modes += mode;
			this->_has_limit = true;
			modeARGs.erase(modeARGs.begin());
			return(0);
		}
		case ('o'): {
			if (modeARGs.empty())
				return (461);
			addOperator(findFromMember(modeARGs[0]));
			modeARGs.erase(modeARGs.begin());
			return (0);
		}
		default:
			return (472);
	}
}

int Channel::delMode(char mode, std::vector<std::string>& modeARGs) {
	size_t pos = this->_modes.find(mode);
	if (pos != std::string::npos) {
		_modes.erase(pos, 1);
		switch (mode) {
		case ('l'): {
			_limit_numeric = -1;
			_limit_string = "";
			_has_limit = false;
			return (0);
		}
		case ('k'): {
			_key = "";
			_key_settings = false;
			return (0);
		}
		case ('i'): {
			_invite_only = false;
			_invited_FD.clear();
			return (0);
		}
		case ('t'): {
			_topic_restricted = false;
			_topic.clear();
			return (0);
		}
		default:
			return (472);
		}
	}
	if (mode == 'o') {
		if (modeARGs.empty())
			return (461);
		Client *delOper;
		delOper = findFromMember(modeARGs[0]);
		if (delOper == NULL)
			return(441);
		if (deleteOperator(delOper) != -1) //Ira: if user isn't an operator (reurning (-1)), just ignore the command
			modeARGs.erase(modeARGs.begin());
	}
	return (0);
}

bool Channel::userInvited(int FD) {
	for (std::vector<int>::iterator it = _invited_FD.begin(); it != _invited_FD.end(); it++) {
		if (*it == FD)
			return (true);
	}
	return (false);
}

void Channel::removeFromInvited(int FD) {
	for (std::vector<int>::iterator it = _invited_FD.begin(); it != _invited_FD.end(); it++) {
		if (*it == FD) {
			_invited_FD.erase(it);
			break;
		}
	}
}

void Channel::setTopic(std::string topic) {
	this->_topic = topic;
}

Client* Channel::findFromMember(std::string nickName) { //Ira: I need it to solve +o, because I did a vector of *Client
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); it++) {
		if ((*it)->getNickname() == nickName)
			return (*it);
	}
	return (NULL);
}