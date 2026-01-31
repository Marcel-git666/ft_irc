#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
private:
  int _fd;             // The socket file descriptor (the "phone line")
  std::string _ipAddr; // The user's IP address (for logging/bans)
  std::string _buffer; // The storage for incoming partial data

public:
  // Constructor & Destructor
  Client(int fd, std::string ip);
  ~Client();

  // Getters
  int getFd() const;
  std::string getIpAddr() const;
  std::string getBuffer() const;

  // Buffer Management (The "Bucket")
  void appendBuffer(std::string data);
  void clearBuffer();

  // Handover Logic (For Irina/Parser)
  // Checks if there is a full command (\r\n) in the buffer
  bool isReady();
  // Extracts the full command and removes it from the buffer
  std::string extractMessage();
};

#endif
