#include "Server.hpp"

std::string Server::GetErrorStr(std::string args, int errorNumber, const Client& client) {
	switch (errorNumber) {
	case (401):
		return(":server 401 " + client.getNickname() + " " + args + " :No such nick/channel\r\n");
	case (403)
		return(":server 403 " + client.getNickname() + " " + args + " :No such channel\r\n");
	case (433):
		return(":server 433 * " + args + " :Nickname is already in use\r\n");
	case (432):
		return(":server 432 * " + args + " :Erroneus nickname\r\n");
	case (464):
		return(":server 464 * :Password incorrect\r\n");
	case (461):
		return(":server 461 USER :Not enough parameters\r\n");
	case (462):
		return(":server 462 * :You may not reregister\r\n");
	}
	return ("");
}