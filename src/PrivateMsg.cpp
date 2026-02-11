#include "Server.hpp"

std::vector<std::string> split(std::string targetsStr, char delimeter) {
	std::vector<std::string> targets;
	std::stringstream ss;
	
}

void Server::sendPrivateMsg(const Client& client, std::string args) {
	std::vector<std::string> targets;
	size_t colonPos = args.find(":");
	std::string targetsStr = args.substr(0, colonPos - 2); //Ira: need to cut space and colon
	std::string message = args.substr(colonPos + 1);
	if (targetsStr.find(',') != std::string::npos)
		targets = split(targetsStr, ',');
	else
		targets[0] = targetsStr;
	
}