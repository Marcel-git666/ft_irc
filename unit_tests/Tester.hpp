#pragma once

#include "../inc/Server.hpp"

class FakeClient : public Client {
public:
	FakeClient(int fd, std::string ip);

	std::vector<std::string> messages;

	void sendMessage(const std::string &msg) const;

	std::string getLastMsg() const;

	~FakeClient();
};

int test_registration(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy);
int test_privat_msg(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy);
int test_join(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy);
int test_chan_logic(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy);