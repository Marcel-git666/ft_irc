#include "Client.hpp"
#include <iostream>

// Constructor: Called when you accept() a new connection
Client::Client(int fd, std::string ip) : _fd(fd), _ipAddr(ip) {
  // Initialize buffer as empty
  _buffer = "";
  _registered = false;
  _hasPassword = false;
  _nickname = "";
  _username = "";
  _realname = "";
}

Client::~Client() {
  // We will close the FD in Server.cpp, so usually nothing to do here.
  // Unless you allocate memory for nickname/channels later.
}

// GETTERS
int Client::getFd() const { return _fd; }

std::string Client::getIpAddr() const { return _ipAddr; }

std::string Client::getBuffer() const { return _buffer; }

bool Client::getRegistered() const { return _registered; }

bool Client::getHasPassword() const { return _hasPassword; }

std::string Client::getNickname() const { return _nickname; }

std::string Client::getUsername() const { return _username; }

std::string Client::getRealname() const { return _realname; }

//SETTERS
void Client::setRegistered() { 
	if (_hasPassword == true && !_nickname.empty() && !_username.empty() && !_realname.empty())
		_registered = true;
}

void Client::setHasPassword() { _hasPassword = true; }

void Client::setNickname(std::string nickname) { _nickname = nickname; }

void Client::setUsername(std::string username) { _username = username; }

void Client::setRealname(std::string realname) { _realname = realname; }

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
  std::string message = _buffer.substr(0, pos - 1); // -1 because cut '\r' as well

  // Remove the message AND the newline from the buffer
  _buffer.erase(0, pos + 1);
  return message;
}


std::ostream& operator<<(std::ostream& out, const Client& client) {
	out << "Client info :\n" << "nickname: " << client.getNickname() << ", realname: " <<
		client.getRealname() << ", username: " << client.getUsername() 
		<< " password (true/false): " << client.getHasPassword();
	return (out);
}
