#pragma once

#include <string>
#include <sys/socket.h> // send()
#include <unistd.h>

class Client {
private:
  int _fd;               // The socket file descriptor (the "phone line")
  std::string _ipAddr;   // The user's IP address (for logging/bans)
  std::string _buffer;   // The storage for incoming partial data
  std::string _nickname; // Ira: should be unique
  std::string _username;
  std::string _realname;
  bool _hasPassword; // Ira: set as false, unles user give us correct password
  bool _registered;  // Ira: set as false, changed if password approwed and nick
                     // is unick, and we have user info (username and realname)
  // OCF - Private to prevent copying
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

  // Buffer Management (The "Bucket")
  void appendBuffer(const std::string &data);
  void clearBuffer();

  // Handover Logic (For Irina/Parser)
  // Checks if there is a full command (\r\n) in the buffer
  bool isReady();
  // Extracts the full command and removes it from the buffer
  std::string extractMessage();

  // Ira: include sending message into client obj, becaouse for unit tests don
  virtual void sendMessage(const std::string &msg) const;
};

std::ostream &operator<<(std::ostream &out, const Client &client);
