#pragma once

#include "Client.hpp"
#include <map>
#include <sstream>
#include <vector>

class Channel {
private:
	std::map<int, std::string> _members;
	std::vector<int> _operatorsFDs;
	std::vector<int> _invited_FD;
	std::string _name;
	std::string _topic;
	std::string _modes;
	std::string _key;
	std::string _limit_string;
	bool _invite_only;
	bool _topic_restricted;
	bool _key_settings;
	int _limit_numeric;

  // OCF - Private to prevent copying
public:
  // Constructor & Destructor
  Channel();
  Channel(const Channel &other);
  Channel &operator=(const Channel &other);
  Channel(Client *client, const std::string &name);
  ~Channel();

  // GETTERS
  std::map<int, std::string> getMembers();
  std::vector<int> getOperators();
  std::string getTopic();
  std::string getChName();
  std::string getModestring();
  std::string getKey();
  std::string getLimitString();
  int getLimitNumeric();
  bool getKeySetting();
  bool getTopicRestr();
  bool getInviteSettings();
  bool isFull();

  // SETTERS
  void setTopic(const std::string &topic);

  void addMember(int clientID, const std::string &clientNickname);
  void addOperator(int clientID);
  int deleteOperator(int clientFD);
  void addInvited(int FD_inv);
  bool userInvited(int FD);
  void removeFromInvited(int FD);

  bool clientIsOperator(int clientFD);
  bool clientIsMember(int clientFD);

  void deleteClient(const std::string &clientNickname);
  void deleteClient(int FD);

  // MODES
  int addMode(char mode, std::vector<std::string> &modeARGs);
  int delMode(char mode, std::vector<std::string> &modeARGs);

  int findFromMember(const std::string &nickName);
};

std::ostream &operator<<(std::ostream &out, const Channel &channel);
