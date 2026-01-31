#include "Client.hpp"
#include <iostream>

// Constructor: Called when you accept() a new connection
Client::Client(int fd, std::string ip) : _fd(fd), _ipAddr(ip) {
  // Initialize buffer as empty
  _buffer = "";
}

Client::~Client() {
  // We will close the FD in Server.cpp, so usually nothing to do here.
  // Unless you allocate memory for nickname/channels later.
}

// GETTERS
int Client::getFd() const { return _fd; }

std::string Client::getIpAddr() const { return _ipAddr; }

std::string Client::getBuffer() const { return _buffer; }

// BUFFER HANDLING (The "Bucket")

// 1. Add raw data to the bucket
void Client::appendBuffer(std::string data) { _buffer += data; }

// 2. Clear the bucket (if needed)
void Client::clearBuffer() { _buffer.clear(); }

// 3. Check if we have a full line (\n)
bool Client::isReady() {
  // Return true if '\n' exists in the buffer
  return _buffer.find('\n') != std::string::npos;
}

// 4. The Handover: Extract one full command
std::string Client::extractMessage() {
  size_t pos = _buffer.find('\n');

  // If no newline, return empty (waiting for more data)
  if (pos == std::string::npos) {
    return "";
  }

  // Extract the substring (0 to pos)
  // We include up to pos. If you want to strip \r\n, handle it here or in
  // parser.
  std::string message = _buffer.substr(0, pos);

  // Remove the message AND the newline from the buffer
  _buffer.erase(0, pos + 1);

  return message;
}
