#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream> // Required for std::ostream
#include <string>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @class Client
 * @brief Represents a single connected IRC client.
 * * Manages client-specific data such as socket file descriptor, network
 * addresses, authentication status, and the incoming data buffer.
 */
class Client {
private:
  int _fd;
  std::string _ipAddr;
  std::string _buffer;
  std::string _nickname;
  std::string _username;
  std::string _realname;
  bool _hasPassword;
  bool _registered;

  // OCF - Private to prevent copying (protects against double socket closing)
  Client(const Client &other);
  Client &operator=(const Client &other);

public:
  // Constructor & Destructor
  Client(int fd, const std::string &ip);
  virtual ~Client();

  // Getters
  int getFd() const;
  std::string getIpAddr() const;
  std::string getBuffer() const;
  bool getHasPassword() const;
  bool getRegistered() const;
  std::string getNickname() const;
  std::string getUsername() const;
  std::string getRealname() const;

  // Setters
  bool setRegistered();
  void setHasPassword();
  void setNickname(const std::string &nickname);
  void setUsername(const std::string &username);
  void setRealname(const std::string &realname);

  // Buffer Management
  void appendBuffer(const std::string &data);
  void clearBuffer();
  bool isReady();
  std::string extractMessage();

  // Network Communication
  virtual void sendMessage(const std::string &msg) const;
};

// Operator overload for debug printing
std::ostream &operator<<(std::ostream &out, const Client &client);

#endif
