#include "Tester.hpp"

int test_join(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy) {
	std::string args;
	int i = 0;

	std::cout << BLUE << "Joining on one channel is testing..." << ENDCOLOR << std::endl;
	/*JOIN on one channel, become an operator*/
	args = "JOIN #alicechan";
	server.executeCMD(args, alice);
	if (alice.getLastMsg() != ":server 366 alice #alicechan :End of /NAMES list\r\n") {
		i++;
		std::cout << RED << "JOIN on channel (message to client): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN on channel (message to client): SUCSESS" << ENDCOLOR << std::endl;
	if (!server.searchChannel("#alicechan"))
		std::cout << RED << "JOIN on channel (add channel to the server): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "JOIN on channel (add channel to the server): SUCSESS" << ENDCOLOR << std::endl;
	if (!server.searchChannel("#alicechan")->clientIsOperator(alice.getFd())){
		i++;
		std::cout << RED << "JOIN on channel (first client in chan is an operator): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN on channel (first client in chan is an operator): SUCSESS" << ENDCOLOR << std::endl;
	/*JOIN on two channels, become an operator of one of them, broadcast joining for another member*/
	std::cout << BLUE << "Joining on two channels in a row is testing..." << ENDCOLOR << std::endl;
	args = "JOIN #alicechan,#bobchan";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 366 bob #bobchan :End of /NAMES list\r\n") {
		i++;
		std::cout << RED << "JOIN on two channel in a raw (message to client): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN on two channel in a raw (message to client): SUCSESS" << ENDCOLOR << std::endl;
	if (alice.getLastMsg() != ":bob!~bob@localhost JOIN #alicechan\r\n") {
		i++;
		std::cout << RED << "JOIN on two channel in a raw (broadcast to other members): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN on two channel in a raw (broadcast to other members): SUCSESS" << ENDCOLOR << std::endl;
	if (!server.searchChannel("#alicechan")->clientIsMember(bob.getFd()) || !server.searchChannel("#bobchan")->clientIsOperator(bob.getFd())) {
		i++;
		std::cout << RED << "JOIN on two channel in a raw (client is member of both channels): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN on two channel in a raw (client is member of both channels): SUCSESS" << ENDCOLOR << std::endl;
	/*JOIN on three channels, become an operator of one of them, broadcast joining for another member*/
	std::cout << BLUE << "Joining on three channel in a row, testing..." << ENDCOLOR << std::endl;
	args = "JOIN #alicechan,#bobchan,#judychan";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 366 judy #judychan :End of /NAMES list\r\n") {
		i++;
		std::cout << RED << "JOIN on two channel in a raw (message to client): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN on three channel in a raw (message to client): SUCSESS" << ENDCOLOR << std::endl;
	if (alice.getLastMsg() != ":judy!~judy@localhost JOIN #alicechan\r\n" || bob.getLastMsg() != ":judy!~judy@localhost JOIN #bobchan\r\n") {
		i++;
		std::cout << RED << "JOIN on three channel in a raw (broadcast to other members): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN on three channel in a raw (broadcast to other members): SUCSESS" << ENDCOLOR << std::endl;
	if (!server.searchChannel("#alicechan")->clientIsMember(judy.getFd()) || !server.searchChannel("#bobchan")->clientIsMember(judy.getFd()) 
		|| !server.searchChannel("#judychan")->clientIsOperator(judy.getFd())) {
		i++;
		std::cout << RED << "JOIN on three channel in a raw (client is member of both channels): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN on three channel in a raw (client is member of all three channels): SUCSESS" << ENDCOLOR << std::endl;
	return (i);
}