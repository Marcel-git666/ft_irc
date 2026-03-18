#include "Tester.hpp"

int test_chan_logic(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy) {
	std::string args;
	int i = 0;

	/*MODE +o*/
	std::cout << BLUE << "Channel mode +o is testing..." << ENDCOLOR << std::endl;
	args = "MODE #alicechan +o bob";
	server.executeCMD(args, alice);
	if (!server.searchChannel("#alicechan")->clientIsOperator(bob.getFd())) {
		std::cout << RED << "MODE +o (one new oper): FAIL" << ENDCOLOR << std::endl;
		i++;
	}
	else
		std::cout << GREEN << "MODE +o (one new oper): SUCSESS" << ENDCOLOR << std::endl;
	if (bob.getLastMsg() != ":alice!~alice@localhost MODE #alicechan +o bob\r\n") {
		i++;
		std::cout << RED << "MODE +o (one new oper, broadcast): FAIL" << ENDCOLOR << std::endl;
	}
		else
		std::cout << GREEN << "MODE +o (one new oper, broadcast): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan -o bob";
	server.executeCMD(args, alice);
	if (server.searchChannel("#alicechan")->clientIsOperator(bob.getFd())) {
		i++;
		std::cout << RED << "MODE -o (delete one oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "MODE -o (delete oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan +oo bob judy";
	server.executeCMD(args, alice);
	if (!server.searchChannel("#alicechan")->clientIsOperator(bob.getFd()) || !server.searchChannel("#alicechan")->clientIsOperator(judy.getFd())) {
		i++;
		std::cout << RED << "MODE +oo (two new oper): FAIL" << ENDCOLOR << std::endl;
	}
		else
		std::cout << GREEN << "MODE +oo (two new oper): SUCSESS" << ENDCOLOR << std::endl;
	if (bob.getLastMsg() != ":alice!~alice@localhost MODE #alicechan +oo bob judy\r\n" 
		|| judy.getLastMsg() != ":alice!~alice@localhost MODE #alicechan +oo bob judy\r\n") {
		i++;
		std::cout << RED << "MODE +oo (two new oper, broadcast): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "MODE +oo (two new oper, broadcast): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan -oo bob judy";
	server.executeCMD(args, alice);
	if (server.searchChannel("#alicechan")->clientIsOperator(bob.getFd()) || server.searchChannel("#alicechan")->clientIsOperator(judy.getFd())) {
		i++;
		std::cout << RED << "MODE -oo (delete two oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "MODE -oo (delete two oper): SUCSESS" << ENDCOLOR << std::endl;
	if (bob.getLastMsg() != ":alice!~alice@localhost MODE #alicechan -oo bob judy\r\n" 
		|| judy.getLastMsg() != ":alice!~alice@localhost MODE #alicechan -oo bob judy\r\n") {
		i++;
		std::cout << RED << "MODE -oo (delete two oper, broadcast): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "MODE -oo (delete two oper, broadcast): SUCSESS" << ENDCOLOR << std::endl;
	
	std::cout << BLUE << "Channel mode +iklt is testing..." << ENDCOLOR << std::endl;
	/*MODE +iklt*/
	args = "MODE #alicechan +iklt key 3";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 482 * #alicechan :You're not channel operator\r\n") {
		i++;
		std::cout << RED << "MODE +iklt (from not an oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "MODE  +iklt (from not an oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan +iklt key";
	server.executeCMD(args, alice);
	if (alice.getLastMsg() != ":server 461 MODE :Not enough parameters\r\n") {
		i++;
		std::cout << RED << "MODE +iklt key (not enough parameters): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "MODE  +iklt key (not enough parameters): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan +iklt key 3";
	server.executeCMD(args, alice);
	if (bob.getLastMsg() != ":alice!~alice@localhost MODE #alicechan +iklt key 3\r\n") {
		i++;
		std::cout << RED << "MODE +iklt (from an oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "MODE  +iklt (from an oper): SUCSESS" << ENDCOLOR << std::endl;
	if (!server.searchChannel("#alicechan")->getInviteSettings() || !server.searchChannel("#alicechan")->getKeySetting()
		|| server.searchChannel("#alicechan")->getLimitNumeric() != 3 || !server.searchChannel("#alicechan")->getTopicRestr()
		|| server.searchChannel("#alicechan")->getKey() != "key") {
		i++;
		std::cout << RED << "MODE +iklt (mode appling): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "MODE  +iklt (mode appling): SUCSESS" << ENDCOLOR << std::endl;
	
	//judy leave channel, test PART command
	std::cout << BLUE << "PART is testing..." << ENDCOLOR << std::endl;
	args = "PART #alicechan :bye";
	server.executeCMD(args, judy);
	if (!server.searchChannel("#alicechan")->clientIsMember(judy.getFd()))
		std::cout << GREEN << "PART (leave chan): SUCSESS" << ENDCOLOR << std::endl;
	else {
		i++;
		std::cout << RED << "PART (leave chan): FAIL " << ENDCOLOR << std::endl;
	}
	if (bob.getLastMsg() != ":judy!~judy@localhost PART #alicechan :bye\r\n" 
		|| alice.getLastMsg() != ":judy!~judy@localhost PART #alicechan :bye\r\n") {
		i++;
		std::cout << RED << "PART (broadcast): FAIL " << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "PART (broadcast): SUCSESS" << ENDCOLOR << std::endl;
	/*Test MODE while not on channel*/
	args = "MODE #alicechan -iklt";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 442 * #alicechan :You're not on that channel\r\n" ) {
		i++;
		std::cout << RED << "MODE from not a chan member: FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "MODE from not a chan member: SUCSESS" << ENDCOLOR << std::endl;

	std::cout << BLUE << "JOIN with different chan modes is testing..." << ENDCOLOR << std::endl;
	/*Test JOIN while restricted*/
	args = "JOIN #alicechan";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 475 * #alicechan :Cannot join channel (+k)\r\n" ) {
		i++;
		std::cout << RED << "JOIN while +k MODE (without key): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN while +k MODE (without key): SUCSESS" << ENDCOLOR << std::endl;
	args = "JOIN #alicechan key";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 473 * #alicechan :Cannot join channel (+i)\r\n" ) {
		i++;
		std::cout << RED << "JOIN while +i MODE (not invited): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN while +i MODE (not invited): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan +l 2";
	server.executeCMD(args, alice);
	/*TEst KICK*/
	std::cout << BLUE << "KICK is testing..." << ENDCOLOR << std::endl;
	args = "KICK #alicechan alice";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 482 * #alicechan :You're not channel operator\r\n") {
		i++;
		std::cout << RED << "KICK (from not a chan oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "KICK (from not a chan oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "KICK #alicechan bob";
	server.executeCMD(args, alice);
	if (bob.getLastMsg() != ":alice!~alice@localhost KICK #alicechan bob\r\n" || alice.getLastMsg() != ":alice!~alice@localhost KICK #alicechan bob\r\n") {
		i++;
		std::cout << RED << "KICK (from a chan oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "KICK (from a chan oper): SUCSESS" << ENDCOLOR << std::endl;

	std::cout << BLUE << "INVITE is testing..." << ENDCOLOR << std::endl;
	/*Test INVITE*/
	args = "INVITE judy #alicechan";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 482 * #alicechan :You're not channel operator\r\n" ) {
		i++;
		std::cout << RED << "INVITE (from not a chan oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "INVITE (from not a chan oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "INVITE smone #alicechan";
	server.executeCMD(args, alice);
	if (alice.getLastMsg() != ":server 401 * smone :No such nick/channel\r\n" ) {
		i++;
		std::cout << RED << "INVITE (no such nick): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "INVITE (no such nick): SUCSESS" << ENDCOLOR << std::endl;
	args = "INVITE judy #alicechan";
	server.executeCMD(args, alice);
	if (judy.getLastMsg() != ":alice!~alice@localhost INVITE judy #alicechan\r\n" ) {
		i++;
		std::cout << RED << "INVITE (from chan oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "INVITE (from chan oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "INVITE judy,bob #alicechan";
	server.executeCMD(args, alice);
	if (judy.getLastMsg() != ":alice!~alice@localhost INVITE judy #alicechan\r\n" || bob.getLastMsg() != ":alice!~alice@localhost INVITE bob #alicechan\r\n") {
		i++;
		std::cout << RED << "INVITE (two clients in row): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "INVITE (two clients in row): SUCSESS" << ENDCOLOR << std::endl;
	args = "JOIN #alicechan key";
	server.executeCMD(args, judy);
	args = "INVITE judy #alicechan";
	server.executeCMD(args, alice);
	if (alice.getLastMsg() != ":server 443 judy #alicechan :is already on channel\r\n") {
		i++;
		std::cout << RED << "INVITE (when client already on chan): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "INVITE (when client already on chan): SUCSESS" << ENDCOLOR << std::endl;
	args = "JOIN #alicechan key";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 471 * #alicechan :Cannot join channel (+l)\r\n") {
		i++;
		std::cout << RED << "JOIN while +l MODE (3d while capacity is 2): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN while +l MODE (3d while capacity is 2): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan +l 3";
	server.executeCMD(args, alice);
	args = "JOIN #alicechan key";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 366 judy #alicechan :End of /NAMES list\r\n") {
		i++;
		std::cout << RED << "JOIN while +ikl MODE (all set): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "JOIN while +ikl MODE (all set): SUCSESS" << ENDCOLOR << std::endl;

	std::cout << BLUE << "TOPIC with different chan modes is testing..." << ENDCOLOR << std::endl;
	/*Set topic*/
	args = "TOPIC #alicechan :topic";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 482 * #alicechan :You're not channel operator\r\n") {
		i++;
		std::cout << RED << "TOPIC while +t MODE (not a chan oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "TOPIC while +t MODE (not a chan oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "JOIN #alicechan key"; //need tojoin after being Kicked
	server.executeCMD(args, bob);
	args = "TOPIC #alicechan :topic";
	server.executeCMD(args, alice);
	if (alice.getLastMsg() != ":alice!~alice@localhost TOPIC #alicechan :topic\r\n"
		|| bob.getLastMsg() != ":alice!~alice@localhost TOPIC #alicechan :topic\r\n" || judy.getLastMsg() != ":alice!~alice@localhost TOPIC #alicechan :topic\r\n") {
		i++;
		std::cout << RED << "TOPIC while +t MODE (chan oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "TOPIC while +t MODE (chan oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan -klit";
	server.executeCMD(args, alice);
	args = "TOPIC #alicechan :judy_topic";
	server.executeCMD(args, judy);
	if (alice.getLastMsg() != ":judy!~judy@localhost TOPIC #alicechan :judy_topic\r\n" 
		|| bob.getLastMsg() != ":judy!~judy@localhost TOPIC #alicechan :judy_topic\r\n" || judy.getLastMsg() != ":judy!~judy@localhost TOPIC #alicechan :judy_topic\r\n") {
		i++;
		std::cout << RED << "TOPIC whithout topic restricted MODE (not a chan oper): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "TOPIC whithout topic restricted MODE (not a chan oper): SUCSESS" << ENDCOLOR << std::endl;
	if (!server.searchChannel("#alicechan")->getInviteSettings() || !server.searchChannel("#alicechan")->getKeySetting()
		|| server.searchChannel("#alicechan")->getLimitNumeric() != 3 || !server.searchChannel("#alicechan")->getTopicRestr()
		|| server.searchChannel("#alicechan")->getKey() != "key")
		std::cout << GREEN << "MODE -iklt (mode deleting): SUCSESS" << ENDCOLOR << std::endl;
	else {
		i++;
		std::cout << RED << "MODE -iklt (mode deleting): FAIL" << ENDCOLOR << std::endl;
	}
	/*PRIVMSG to the chan*/
	std::cout << BLUE << "PRIVMSG with different chan modes is testing..." << ENDCOLOR << std::endl;
	args = "PRIVMSG #alicechan :Hi";
	server.executeCMD(args, judy);
	if (alice.getLastMsg() != ":judy!~judy@localhost PRIVMSG #alicechan :Hi\r\n" 
		|| bob.getLastMsg() != ":judy!~judy@localhost PRIVMSG #alicechan :Hi\r\n") {
		i++;
		std::cout << RED << "PRIVMSG to the chan: FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "PRIVMSG to the chan: SUCSESS" << ENDCOLOR << std::endl;
	args = "PRIVMSG #mychan :Hi";
	server.executeCMD(args, judy);
	if (alice.getLastMsg() != ":judy!~judy@localhost PRIVMSG #alicechan :Hi\r\n" 
		|| bob.getLastMsg() != ":judy!~judy@localhost PRIVMSG #alicechan :Hi\r\n" || judy.getLastMsg() != ":server 403 * #mychan :Hi :No such channel\r\n") {
		i++;
		std::cout << RED << "PRIVMSG to the chan (chan does not exist): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "PRIVMSG to the chan (chan does not exist): SUCSESS" << ENDCOLOR << std::endl;
	return (i);
}