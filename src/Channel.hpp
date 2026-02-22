#pragma once

#include "Client.hpp"
#include <vector>

class Channel {
private:
	std::vector<Client *> _members;
	std::vector<Client *> _operators;
	std::string _name;
	std::string _topic;

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
	Client* getClientFromChan(int FD);

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
};

std::ostream& operator<<(std::ostream& out, const Channel& channel);
