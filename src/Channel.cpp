#include "../inc/Channel.hpp"

// Default constructor initializes an empty channel with default modes.
Channel::Channel()
    : _members(), _operatorsFDs(), _invited_FD(), _name(""), _topic(""),
      _modes(""), _key(""), _limit_string(""), _invite_only(false),
      _topic_restricted(false), _key_settings(false), _limit_numeric(-1) {}

// Parameterized constructor creates a new channel and sets the creator as the
// first member and operator.
Channel::Channel(Client *client, const std::string &name)
    : _invited_FD(), _name(name), _topic(""), _modes(""), _key(""),
      _limit_string(""), _invite_only(false), _topic_restricted(false),
      _key_settings(false), _limit_numeric(-1) {
  addMember(client->getFd(), client->getNickname());
  addOperator(client->getFd());
}

// Copy constructor (Required for storing Channels in std::map).
Channel::Channel(const Channel &other) { *this = other; }

// Assignment operator (Required for storing Channels in std::map).
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
  return *this;
}

Channel::~Channel() {}

// --- Getters ---

std::map<int, std::string> Channel::getMembers() { return _members; }
std::vector<int> Channel::getOperators() { return _operatorsFDs; }
std::string Channel::getTopic() { return _topic; }
std::string Channel::getChName() { return _name; }
std::string Channel::getModestring() { return _modes; }
std::string Channel::getKey() { return _key; }
std::string Channel::getLimitString() { return _limit_string; }
int Channel::getLimitNumeric() { return _limit_numeric; }
bool Channel::getKeySetting() { return _key_settings; }
bool Channel::getTopicRestr() { return _topic_restricted; }
bool Channel::getInviteSettings() { return _invite_only; }

// Checks if the channel has reached its user limit (mode +l).
bool Channel::isFull() {
  if (_limit_numeric != -1 && (int)_members.size() >= _limit_numeric)
    return true;
  return false;
}

// --- Setters and Core Logic ---

void Channel::setTopic(const std::string &topic) { _topic = topic; }

// Adds a user to the channel members list.
void Channel::addMember(int clientID, const std::string &clientNickname) {
  _members[clientID] = clientNickname;
}

// Grants operator privileges (+o) to a specific user.
void Channel::addOperator(int clientID) { _operatorsFDs.push_back(clientID); }

// Revokes operator privileges (-o) from a specific user.
int Channel::deleteOperator(int clientFD) {
  for (std::vector<int>::iterator itOp = _operatorsFDs.begin();
       itOp != _operatorsFDs.end(); ++itOp) {
    if (*itOp == clientFD) {
      _operatorsFDs.erase(itOp);
      return 0;
    }
  }
  return -1;
}

// Adds a user to the whitelist of invited users (mode +i).
void Channel::addInvited(int FD_inv) { _invited_FD.push_back(FD_inv); }

// Checks if a user is in the invited whitelist.
bool Channel::userInvited(int FD) {
  for (std::vector<int>::iterator it = _invited_FD.begin();
       it != _invited_FD.end(); ++it) {
    if (*it == FD)
      return true;
  }
  return false;
}

// Removes a user from the invited whitelist (usually after they successfully
// join).
void Channel::removeFromInvited(int FD) {
  for (std::vector<int>::iterator it = _invited_FD.begin();
       it != _invited_FD.end(); ++it) {
    if (*it == FD) {
      _invited_FD.erase(it);
      break;
    }
  }
}

// Checks if a specific user has operator privileges.
bool Channel::clientIsOperator(int clientFD) {
  for (std::vector<int>::iterator it = _operatorsFDs.begin();
       it != _operatorsFDs.end(); ++it) {
    if (*it == clientFD)
      return true;
  }
  return false;
}

// Checks if a specific user is currently in the channel.
bool Channel::clientIsMember(int clientFD) {
  return _members.find(clientFD) != _members.end();
}

// Completely removes a user from the channel (members, operators, and invites).
void Channel::deleteClient(int FD) {
  _members.erase(FD);
  deleteOperator(FD);
  removeFromInvited(FD);
}

// --- Mode Management ---

// Applies a positive mode (+i, +k, +t, +l, +o) to the channel.
// Returns IRC error codes (461, 441, 472) if validation fails.
int Channel::addMode(char mode, std::vector<std::string> &modeARGs) {
  switch (mode) {
  case ('i'):
    if (_modes.find(mode) == std::string::npos)
      _modes += mode;
    _invite_only = true;
    return 0;
  case ('k'):
    if (modeARGs.empty())
      return 461; // ERR_NEEDMOREPARAMS
    if (_modes.find(mode) == std::string::npos)
      _modes += mode;
    _key_settings = true;
    _key = modeARGs[0];
    modeARGs.erase(modeARGs.begin());
    return 0;
  case ('t'):
    if (_modes.find(mode) == std::string::npos)
      _modes += mode;
    _topic_restricted = true;
    return 0;
  case ('l'): {
    if (modeARGs.empty())
      return 461; // ERR_NEEDMOREPARAMS
    std::stringstream ss(modeARGs[0]);
    if (!(ss >> _limit_numeric) || _limit_numeric < 0)
      return 461;
    _limit_string = modeARGs[0];
    if (_modes.find(mode) == std::string::npos)
      _modes += mode;
    modeARGs.erase(modeARGs.begin());
    return 0;
  }
  case ('o'): {
    if (modeARGs.empty())
      return 461; // ERR_NEEDMOREPARAMS
    int newOperFD = findFromMember(modeARGs[0]);
    if (newOperFD == 0)
      return 441; // ERR_USERNOTINCHANNEL
    if (!clientIsOperator(newOperFD))
      addOperator(newOperFD);
    modeARGs.erase(modeARGs.begin());
    return 0;
  }
  default:
    return 472; // ERR_UNKNOWNMODE
  }
}

// Removes a mode (-i, -k, -t, -l, -o) from the channel and resets its specific
// rule.
int Channel::delMode(char mode, std::vector<std::string> &modeARGs) {
  size_t pos = _modes.find(mode);
  if (pos != std::string::npos) {
    _modes.erase(pos, 1);
    switch (mode) {
    case ('l'):
      _limit_numeric = -1;
      _limit_string = "";
      return 0;
    case ('k'):
      _key = "";
      _key_settings = false;
      return 0;
    case ('i'):
      _invite_only = false;
      _invited_FD.clear();
      return 0;
    case ('t'):
      _topic_restricted = false;
      return 0; // Note: topic string is kept, just restriction is lifted
    default:
      return 472; // ERR_UNKNOWNMODE
    }
  }

  if (mode == 'o') {
    if (modeARGs.empty())
      return 461; // ERR_NEEDMOREPARAMS
    int delOper = findFromMember(modeARGs[0]);
    if (delOper == 0)
      return 441; // ERR_USERNOTINCHANNEL
    if (deleteOperator(delOper) != -1) {
      modeARGs.erase(modeARGs.begin());
    }
  }
  return 0;
}

// --- Utils ---

// Looks up a member's FD by their nickname. Returns 0 if not found.
int Channel::findFromMember(const std::string &nickName) {
  for (std::map<int, std::string>::iterator it = _members.begin();
       it != _members.end(); ++it) {
    if (it->second == nickName)
      return it->first;
  }
  return 0;
}
