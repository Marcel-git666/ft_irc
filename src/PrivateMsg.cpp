#include "Server.hpp"

std::vector<std::string> split(const std::string& targetsStr, char delimeter) {
	std::vector<std::string> targets;
	size_t start = 0;
	size_t pos = targetsStr.find(delimeter);
	while (pos != std::string::npos) {
		targets.push_back(targetsStr.substr(start, pos - start));
		start = pos + 1;
		pos = targetsStr.find(delimeter, start);
	}
	targets.push_back(targetsStr.substr(start));
	return (targets);
	
}

void Server::sendPrivateMsg(const Client& sender, std::string args) {
	std::vector<std::string> targets;
	size_t colonPos = args.find(":");
	std::string targetsStr = args.substr(0, colonPos - 2); //Ira: need to cut space and colon
	std::string msg = args.substr(colonPos + 1);
	targets = split(targetsStr, ',');
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); it++) {
		int FD = clientFdsearch(*it);
		if(FD) {
			std::string message = ":" + sender.getNickname() + "!" +
                sender.getUsername() + "@localhost PRIVMSG " +
                *it + " :" + msg + "\r\n";
			send(FD, msg.c_str(), msg.size(), 0);
		}
	}
	
}