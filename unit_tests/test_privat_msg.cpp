#include "Tester.hpp"

int test_privat_msg(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy) {
	std::string args;
	int i = 0;

	std::cout << BLUE << "Private messages testing..." << ENDCOLOR << std::endl;
	args = "PRIVMSG bob :Hi bob!";
	server.executeCMD(args, alice);
	if (bob.getLastMsg() != ":alice!~alice@localhost PRIVMSG bob :Hi bob!\r\n") {
		i++;
		std::cout << RED << "PRIVMSG for one user: FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "PRIVMSG for one user: SUCSESS" << ENDCOLOR << std::endl;
	args = "PRIVMSG bob,judy :Hello!";
	server.executeCMD(args, alice);
	if (bob.getLastMsg() != ":alice!~alice@localhost PRIVMSG bob :Hello!\r\n" || judy.getLastMsg() != ":alice!~alice@localhost PRIVMSG judy :Hello!\r\n") {
		i++;
		std::cout << RED << "PRIVMSG for two user in a raw: FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "PRIVMSG for two user in a raw: SUCSESS" << ENDCOLOR << std::endl;
	return (i);
}