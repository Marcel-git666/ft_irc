#pragma once

#include "Client.hpp"
#include <vector>
#include <sstream>

class Channel {
private:
	std::vector<Client *> _members;
	std::vector<Client *> _operators;
	std::vector<int> _invited_FD;
	std::string _name;
	std::string _topic;
	std::string _modes;
	std::string _key;
	std::string _limit_string;
	bool _invite_only;
	bool _topic_restrictions;
	bool _key_settings;
	bool _has_limit;
	int _limit_numeric;

	// OCF - Private to prevent copying
public:
	// Constructor & Destructor
	Channel();
	Channel(const Channel &other);
	Channel &operator=(const Channel &other);
	Channel(Client *client, std::string name);
	~Channel();

	//GETTERS
	std::vector<Client *> getMembers();
	std::vector<Client *> getOperators();
	std::string getTopic();
	std::string getChName();
	std::string getModestring();
	std::string getKey();
	std::string getLimitString();
	int getLimitNumeric();
	Client* getClientFromChan(int FD);
	bool getKeySetting();
	bool getTopicRestr();
	bool getInviteSettings();
	bool isFull();

	//SETTERS
	void setTopic(std::string topic);

	void addMember(Client *newMember);
	void addOperator(Client *newOper);
	void addInvited(int FD_inv);
	bool userInvited(int FD);
	void removeFromInvited(int FD);

	bool clientIsOperator(Client *client);
	bool clientIsMember(Client *client);
	bool clientIsMember(int FD);
	bool clientIsMember(std::string clientNickname);

	void deleteClient(std::string clientNickname);
	void deleteClient(int FD);

	//MODES
	int addMode(char mode, std::vector<std::string>& modeARGs);
	bool delMode(char mode);
};

std::ostream& operator<<(std::ostream& out, const Channel& channel);
