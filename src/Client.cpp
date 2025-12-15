#include "Client.hpp"

Client::Client() : _fd(-1) {}

Client::~Client() {}

void Client::setNickname(std::string const &name) {
    _nickname = name;
}

int Client::getFd() const {
    return _fd;
}