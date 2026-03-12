#pragma once

#include "Server.hpp"

class FakeClient : public Client {
public:
	FakeClient(int fd, std::string ip);

	std::vector<std::string> messages;

	void sendMessage(const std::string &msg) const;

	std::string getLastMsg() const;

	~FakeClient();

};

void test_registration(Server &server, FakeClient &alice, FakeClient &bob);