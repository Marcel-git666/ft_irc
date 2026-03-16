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

void test_registration(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy);
void test_privat_msg(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy);
void test_join(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy);
void test_chan_logic(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy);