#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include <vector>
#include <string>

class Channel {
    private:
        std::string             _name;
        std::vector<Client *>   _clients;

    public:
        Channel(std::string name);
        ~Channel();

        void addClient(Client *newClient);
};

#endif