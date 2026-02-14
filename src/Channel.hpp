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

	//SETTERS
	void setTopic(std::string topic);

	void addMember(Client *newMember);
	void addOperator(Client *newOper);


};

std::ostream& operator<<(std::ostream& out, const Channel& channel);
