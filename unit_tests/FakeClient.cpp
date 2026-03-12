#include "../inc/Tester.hpp"

FakeClient::FakeClient(int fd, std::string ip) : Client(fd, ip) {}

void FakeClient::sendMessage(const std::string &msg) const {
	const_cast<FakeClient*>(this)->messages.push_back(msg);
}

std::string FakeClient::getLastMsg() const {
	return (messages.back());
}

FakeClient::~FakeClient() {}
