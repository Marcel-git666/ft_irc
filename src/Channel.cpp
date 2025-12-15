#include "Channel.hpp"

Channel::Channel(std::string name) : _name(name) {}

Channel::~Channel() {}

void Channel::addClient(Client *newClient) {
    _clients.push_back(newClient);
}