#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
    private:
        int             _fd;
        std::string     _nickname;

    public:
        Client(); // Simplified for now
        ~Client();

        void setNickname(std::string const &name);
        int getFd() const;
};

#endif