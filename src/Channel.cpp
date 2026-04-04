#include "../inc/Channel.hpp"

Channel::Channel()
    : _members(), _operatorsFDs(), _invited_FD(), _name(""), _topic(""),
      _modes(""), _key(""), _limit_string("") {
  _invite_only = false;
  _topic_restricted = false;
  _key_settings = false;
  _limit_numeric = -1;
  _has_limit = false;
}
Channel::Channel(Client *client, const std::string &name)
    : _invited_FD(), _name(name), _topic(""), _modes(""), _key(""),
      _limit_string("") {

  addMember(client->getFd(), client->getNickname());
  addOperator(client->getFd());
  _invite_only = false;
  _topic_restricted = false;
  _key_settings = false;
  _has_limit = false;
  _limit_numeric = -1;
}

Channel::Channel(const Channel &other) {
  _members = other._members;
  _operatorsFDs = other._operatorsFDs;
  _invited_FD = other._invited_FD;
  _name = other._name;
  _topic = other._topic;
  _modes = other._modes;
  _key = other._key;
  _invite_only = other._invite_only;
  _topic_restricted = other._topic_restricted;
  _key_settings = other._key_settings;
  _has_limit = other._has_limit;
  _limit_numeric = other._limit_numeric;
  _limit_string = other._limit_string;
}

Channel &Channel::operator=(const Channel &other) {
  if (this != &other) {
    _members = other._members;
    _operatorsFDs = other._operatorsFDs;
    _invited_FD = other._invited_FD;
    _name = other._name;
    _topic = other._topic;
    _modes = other._modes;
    _key = other._key;
    _invite_only = other._invite_only;
    _topic_restricted = other._topic_restricted;
    _key_settings = other._key_settings;
    _limit_numeric = other._limit_numeric;
    _limit_string = other._limit_string;
  }
  return (*this);
}

Channel::~Channel() {}

void Channel::addMember(int clientID, const std::string &clientNickname) {
  _members[clientID] = clientNickname;
}

void Channel::addOperator(int clientID) { _operatorsFDs.push_back(clientID); }

int Channel::deleteOperator(int clientFD) {
  std::vector<int>::iterator itOp;
  for (itOp = _operatorsFDs.begin(); itOp != _operatorsFDs.end(); itOp++) {
    if (*itOp == clientFD) {
      _operatorsFDs.erase(itOp);
      return 0;
    }
  }
  return (-1);
}

void Channel::addInvited(int FD_inv) { _invited_FD.push_back(FD_inv); }

bool Channel::clientIsOperator(int clientFD) {
  for (std::vector<int>::iterator it = _operatorsFDs.begin();
       it != _operatorsFDs.end(); it++) {
    if (*it == clientFD)
      return (true);
  }
  return (false);
}

bool Channel::clientIsMember(int clientFD) {
  for (std::map<int, std::string>::iterator it = _members.begin();
       it != _members.end(); it++) {
    if (it->first == clientFD)
      return (true);
  }
  return (false);
}

std::map<int, std::string> Channel::getMembers() { return (_members); }

std::vector<int> Channel::getOperators() { return (_operatorsFDs); }

std::string Channel::getTopic() { return (_topic); }

std::string Channel::getChName() { return (_name); }

std::string Channel::getModestring() { return (this->_modes); }

std::string Channel::getKey() { return (this->_key); }

std::string Channel::getLimitString() { return (this->_limit_string); }

int Channel::getLimitNumeric() { return (this->_limit_numeric); }

bool Channel::getKeySetting() { return (this->_key_settings); }

bool Channel::getTopicRestr() { return (this->_topic_restricted); }

bool Channel::getInviteSettings() { return (this->_invite_only); }

bool Channel::isFull() {
  if ((int)_members.size() == _limit_numeric)
    return (true);
  return (false);
}

void Channel::deleteClient(int FD) {
  for (std::map<int, std::string>::iterator it = _members.begin();
       it != _members.end(); it++) {
    if (it->first == FD) {
      _members.erase(FD);
      break;
    }
  }
  for (std::vector<int>::iterator itOp = _operatorsFDs.begin();
       itOp != _operatorsFDs.end(); itOp++) {
    if (*itOp == FD) {
      _operatorsFDs.erase(itOp);
      break;
    }
  }
  for (std::vector<int>::iterator itInv = _invited_FD.begin();
       itInv != _invited_FD.end(); itInv++) {
    if (*itInv == FD) {
      _invited_FD.erase(itInv);
      break;
    }
  }
}

int Channel::addMode(char mode, std::vector<std::string> &modeARGs) {
  switch (mode) {
  case ('i'):
    this->_modes += mode;
    this->_invite_only = true;
    return (0);
  case ('k'):
    if (modeARGs.empty()) // Ira: if no argument from user
      return (461);
    if (_modes.find(mode) ==
        std::string::npos) // Ira: if we have this mode for channel we don't
                           // need to add it to the string, we just need to
                           // change the arg
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
    if (modeARGs.empty()) // Ira: if no argument from user
      return (461);
    std::stringstream ss(modeARGs[0]);
    if (!(ss >> this->_limit_numeric) || _limit_numeric < 0)
      return (461); // Ira: IRC does't have any other error if args is incorrect
    _limit_string = modeARGs[0]; // Ira: if we have proper arg for int then we
                                 // can wright down to string
    if (_modes.find(mode) ==
        std::string::npos) // Ira: if we have this mode for channel we don't
                           // need to add it to the string, we just need to
                           // change the arg
      this->_modes += mode;
    this->_has_limit = true;
    modeARGs.erase(modeARGs.begin());
    return (0);
  }
  case ('o'): {
    if (modeARGs.empty())
      return (461);
    int newOperFD = 0;
    newOperFD = findFromMember(modeARGs[0]);
    if (newOperFD == 0)
      return (441);
    addOperator(newOperFD);
    modeARGs.erase(modeARGs.begin());
    return (0);
  }
  default:
    return (472);
  }
}

int Channel::delMode(char mode, std::vector<std::string> &modeARGs) {
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
    int delOper;
    delOper = findFromMember(modeARGs[0]);
    if (delOper == 0)
      return (441);
    if (deleteOperator(delOper) !=
        -1) // Ira: if user isn't an operator (returning (-1)), just ignore the
            // command
      modeARGs.erase(modeARGs.begin());
  }
  return (0);
}

bool Channel::userInvited(int FD) {
  for (std::vector<int>::iterator it = _invited_FD.begin();
       it != _invited_FD.end(); it++) {
    if (*it == FD)
      return (true);
  }
  return (false);
}

void Channel::removeFromInvited(int FD) {
  for (std::vector<int>::iterator it = _invited_FD.begin();
       it != _invited_FD.end(); it++) {
    if (*it == FD) {
      _invited_FD.erase(it);
      break;
    }
  }
}

void Channel::setTopic(const std::string &topic) { this->_topic = topic; }

int Channel::findFromMember(
    const std::string
        &nickName) { // Ira: I need it to solve +o, because I did a
                     // vector of *Client
  for (std::map<int, std::string>::iterator it = _members.begin();
       it != _members.end(); it++) {
    if (it->second == nickName)
      return (it->first);
  }
  return (0);
}
