#include "../inc/Client.hpp"

// Initializes the client with its file descriptor, IP address, and default
// states.
Client::Client(int fd, const std::string &ip)
    : _fd(fd), _ipAddr(ip), _buffer(""), _nickname(""), _username(""),
      _realname(""), _hasPassword(false), _registered(false) {}

Client::~Client() {}

// --- Getters ---
int Client::getFd() const { return _fd; }
std::string Client::getIpAddr() const { return _ipAddr; }
std::string Client::getBuffer() const { return _buffer; }
bool Client::getRegistered() const { return _registered; }
bool Client::getHasPassword() const { return _hasPassword; }
std::string Client::getNickname() const { return _nickname; }
std::string Client::getUsername() const { return _username; }
std::string Client::getRealname() const { return _realname; }

// --- Setters ---

// Checks if all required information is provided and sets the registered flag.
bool Client::setRegistered() {
  if (_hasPassword && !_nickname.empty() && !_username.empty() &&
      !_realname.empty()) {
    _registered = true;
  }
  return _registered;
}

void Client::setHasPassword() { _hasPassword = true; }
void Client::setNickname(const std::string &nickname) { _nickname = nickname; }
void Client::setUsername(const std::string &username) { _username = username; }
void Client::setRealname(const std::string &realname) { _realname = realname; }

// --- Buffer Management ---

// Appends raw data received from the socket to the client's internal buffer.
void Client::appendBuffer(const std::string &data) { _buffer += data; }

// Clears the entire buffer.
void Client::clearBuffer() { _buffer.clear(); }

// Checks if the buffer contains a complete IRC message (terminated by '\n').
bool Client::isReady() { return _buffer.find('\n') != std::string::npos; }

// Extracts a single complete message from the buffer and safely removes it.
std::string Client::extractMessage() {
  size_t pos = _buffer.find('\n');

  if (pos == std::string::npos) {
    return "";
  }

  std::string message = "";
  // Safely extract the message. If '\r' is present right before '\n', we cut it
  // out. Otherwise, we just cut exactly up to '\n' to prevent memory access
  // errors.
  if (pos > 0 && _buffer[pos - 1] == '\r') {
    message = _buffer.substr(0, pos - 1);
  } else {
    message = _buffer.substr(0, pos);
  }

  // Remove the extracted message AND the newline character from the buffer.
  _buffer.erase(0, pos + 1);
  return message;
}

// --- Network Communication ---

// Sends a raw string message directly to the client's socket.
void Client::sendMessage(const std::string &msg) const {
  send(_fd, msg.c_str(), msg.size(), 0);
}

// Overloads the stream insertion operator for easy debugging output.
std::ostream &operator<<(std::ostream &out, const Client &client) {
  out << "Client info :\n"
      << "nickname: " << client.getNickname()
      << ", realname: " << client.getRealname()
      << ", username: " << client.getUsername()
      << ", password OK: " << (client.getHasPassword() ? "yes" : "no")
      << ", registered: " << (client.getRegistered() ? "yes" : "no")
      << ", IP address: " << client.getIpAddr();
  return out;
}
