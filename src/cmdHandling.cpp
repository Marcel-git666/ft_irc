#include "Server.hpp"

//Ira: execute commands
bool Server::executeCMD(std::string cmd, std::string args, Client& client) {
	if (cmd == "PASS") {
		if (args != _password && !_password.empty()) {
			if (DEBUG)
				std::cout << RED << "Password doesn't match" << ENDCOLOR << std::endl;
			sendError(args, 464, client);
			return (false); //Ira: returned here because if password is wrong we don't accept anything else and immediately disconnect this client
		}
		client.setHasPassword();
		if (DEBUG)
			std::cout << GREEN << "Client FD " << client.getFd() << " has input correct password" << ENDCOLOR << std::endl;
	}
	else if (cmd == "NICK") {
		std::string checkNick = checkNickname(args);
		if (checkNick != "ok") {
			if (checkNick == "433")
				sendError(args, 433, client);
			else if (checkNick == "432")
				sendError(args, 432, client);
			return (true);
		}
		else {
			if (client.getNickname() != "") {
				// [TO DO] (if NICK changes, need to send info to other users in channels)
				std::string oldNick = client.getNickname();
				sendMsgToClient(":" + oldNick + "!" + client.getUsername() + "@localhost NICK :" + args + "\r\n", client);
				if (DEBUG)
					std::cout << GREEN << "Nickname of client FD " << client.getFd() << " changed to: " << args << ENDCOLOR << std::endl;
			}
			else {
				if (DEBUG)
					std::cout << GREEN << "Nickname of new client FD " << client.getFd() << ": " << args << ENDCOLOR << std::endl;
			}
			client.setNickname(args);
		}
		if (!client.getRegistered())
			if (!registerClient(client))
				return (false);
	}
	else if (cmd == "USER") {
		if (!client.getRegistered()) {
			//Ira: USER command looks like USER <username> 0 * :<realname>
			//clients usually use for username and realname the nickname, if it's not set by user
			//to expect an empty username is an extra safety, but we have error massage for it so I included it
			std::string username = args.substr(0, args.find(' ')); //find first arg
			if (username.empty())
				sendError(args, 461, client);
			if (username.length() > USERLEN) {
				username = username.substr(0, USERLEN);
				//Ira: IRCstandard: "If this length is advertised, the username 
				//MUST be silently truncated to the given length before being used." 
			}
			if (username[0] != '~')
				username.insert(0, 1, '~');
				//Ira: IRCstandard: "... the username provided by the client SHOULD be prefixed 
				// by a tilde ('~', 0x7E) to show that this value is user-set."
			client.setUsername(username);
			std::cout << GREEN << "Client FD " << client.getFd() << " username is " << client.getUsername() << ENDCOLOR << std::endl;
			std::string realname = args.substr(args.find(':') + 1);
			client.setRealname(realname);
			std::cout << GREEN << "Client FD " << client.getFd() << " realname is " << client.getRealname() << ENDCOLOR << std::endl;
			if (!registerClient(client)) //Ira: registration if wasn't
				return (false); // we need tu return smth for the case when client can't be registere because of password absence
		}
		else {
			sendError(args, 462, client);
			if (DEBUG)
				std::cout << RED << "Client FD" << client.getFd() << "is already registered" << ENDCOLOR << std::endl;
			return (true); 
		}
	}
	else if (cmd == "PING") {
		std::string token = args.substr(args.find(':') + 1); //token should be the same as recieved by client
		//[ToDo] From Copilot appeared that it's not obligated to have ":", whithout it also should work
		sendPong(client, token);
		if (DEBUG)
			std::cout << GREEN << "PING command was accepted and answered" << ENDCOLOR << std::endl;
	}
	else {
		if (client.getRegistered()) {
			if (cmd == "PRIVMSG") {
				if (args[0] != '#')
					sendPrivateMsg(client, args);
				else
					sendToChannel(client, args);
			}
			else if (cmd == "JOIN") {
				if (args[0] != '#')
					sendError(args, 403, client);
				else
					connectToChannel(&client, args);
			}
			// else if (cmd == "KICK") {
			// 	kickOutOfChannel(&client, args);
			// }
		}
	}
	return (true);
}

//Ira: extract command, and cut the message to the args
std::string Server::extractCMD(std::string& args) {
	size_t pos = args.find(' ');
	std::string command = args.substr(0, pos);
	std::cout << "Command from client: " << command;
	args = args.substr(pos + 1, args.length());
	std::cout << " ARGS for this command: " << args << "." << std::endl;
	return command;
}

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

void Server::sendPong(const Client& client, std::string token) {
	sendMsgToClient(":server PONG " + token + "\r\n", client);
}

void Server::sendError(std::string args, int errorNumber, const Client& client) {
	std::string err = GetErrorStr(args, errorNumber, client);
	send(client.getFd(), err.c_str(), err.size(), 0);
	if (DEBUG)
		std::cout << PINK << "Error " << errorNumber << " for client FD " << client.getFd() << " has been sent!" << ENDCOLOR << std::endl;
}

void Server::sendMsgToClient(std::string msg, const Client& client) {
	send(client.getFd(), msg.c_str(), msg.size(), 0);
	if (DEBUG)
		std::cout << GREEN << "Message from server for client FD " << client.getFd() << " has been sent!" << ENDCOLOR << std::endl;
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

int Server::clientFdsearch(std::string nickName) {
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second->getNickname() == nickName)
			return it->first;   // return the fd (the key)
	}
	return (-1);
}
