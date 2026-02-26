#pragma once

#include "Client.hpp"
#include <vector>

class Channel {
private:
	std::vector<Client *> _members;
	std::vector<Client *> _operators;
	std::vector<Client *> _invited;
	std::string _name;
	std::string _topic;
	std::string _modes;
	std::string _key;
	bool _invite_only;
	bool _topic_restrictions;
	bool _key_settings;
	int _user_limit;

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
	Client* getClientFromChan(int FD);
	bool getKeySetting();
	bool getTopicRestr();
	bool getInviteSettings();
	int getUserLimit();


	//SETTERS
	void setTopic(std::string topic);

	void addMember(Client *newMember);
	void addOperator(Client *newOper);

	bool clientIsOperator(Client *client);
	bool clientIsMember(Client *client);
	bool clientIsMember(int FD);
	bool clientIsMember(std::string clientNickname);

	void deleteClient(std::string clientNickname);
	void deleteClient(int FD);

	//MODES
	bool addMode(char mode);
	bool delMode(char mode);
};

std::ostream& operator<<(std::ostream& out, const Channel& channel);
