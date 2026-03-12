#include "../inc/Server.hpp"

//Ira: check Nickname on Uniqueness and rules
std::string Server::checkNickname(std::string arg) {
	if (!_clients.empty()) {
		for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
			if (it->second->getNickname() == arg)
				return ("433");
		}
	}
	if (isdigit(arg[0]) || arg.find(':') != std::string::npos
		|| arg.find('#') != std::string::npos || arg.find(' ') != std::string::npos)
			return("432");
	return ("ok");
}

bool Server::registerClient(Client& client) {
	if (!_password.empty() && !client.getHasPassword()) {
		sendError(client.getNickname(), 464, client);
		return (false);
	}
	if (client.setRegistered()) { //Ira: registered if all set, if not return false in next if condition, all messages about errors have been sent before
		std::string msg = ":server 001 " + client.getNickname() + " :Welcome to the IRC Network, " + client.getNickname() + "\r\n";
		sendMsgToClient(msg, client);
		//Ira: messages below isn't obligated, but I did it to be more like the original IRC protocol
		msg = ":server 002 :Your host is 42_ircserv, running version 01\r\n";
		sendMsgToClient(msg, client);
		msg = ":server 003 :This server was created " + _creationTime + "\r\n";
		sendMsgToClient(msg, client);
		// format: 004 <nick> <servername> <version> <usermodes> <channelmodes>
		// usermodes = o -operators 
		// channelmodes = i  (invite-only); t  (topic restricted); k  (key/password); o  (channel operator); l  (user limit)
		msg = ":server 004 " + client.getNickname() + " 42_ircserv 1.0 o itkol\r\n"; 
		sendMsgToClient(msg, client);
		if (DEBUG) {
			std::cout << GREEN << "Client FD " << client.getFd() << " has been registered" << ENDCOLOR << std::endl;
			std::cout << client << std::endl;
		}
	}
	else {
		if (DEBUG) 
			std::cout << RED << "Client FD " << client.getFd() << " couldn't be registered" << ENDCOLOR << std::endl;
	}
	return (true);
}