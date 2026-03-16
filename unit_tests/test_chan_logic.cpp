#include "Tester.hpp"

void test_chan_logic(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy) {
	std::string args;

	/*MODE +o*/
	std::cout << BLUE << "Channel mode +o is testing..." << ENDCOLOR << std::endl;
	args = "MODE #alicechan +o bob";
	server.executeCMD(args, alice);
	if (!server.searchChannel("#alicechan")->clientIsOperator(bob.getFd()))
		std::cout << RED << "MODE +o (one new oper): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE +o (one new oper): SUCSESS" << ENDCOLOR << std::endl;
	if (bob.getLastMsg() != ":alice!~alice@localhost MODE #alicechan +o bob\r\n")
		std::cout << RED << "MODE +o (one new oper, broadcast): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE +o (one new oper, broadcast): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan -o bob";
	server.executeCMD(args, alice);
	if (server.searchChannel("#alicechan")->clientIsOperator(bob.getFd()))
		std::cout << RED << "MODE -o (delete one oper): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE -o (delete oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan +oo bob judy";
	server.executeCMD(args, alice);
	if (!server.searchChannel("#alicechan")->clientIsOperator(bob.getFd()) || !server.searchChannel("#alicechan")->clientIsOperator(judy.getFd()))
		std::cout << RED << "MODE +oo (two new oper): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE +oo (two new oper): SUCSESS" << ENDCOLOR << std::endl;
	if (bob.getLastMsg() != ":alice!~alice@localhost MODE #alicechan +oo bob judy\r\n" 
		|| judy.getLastMsg() != ":alice!~alice@localhost MODE #alicechan +oo bob judy\r\n")
		std::cout << RED << "MODE +oo (two new oper, broadcast): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE +oo (two new oper, broadcast): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan -oo bob judy";
	server.executeCMD(args, alice);
	if (server.searchChannel("#alicechan")->clientIsOperator(bob.getFd()) || server.searchChannel("#alicechan")->clientIsOperator(judy.getFd()))
		std::cout << RED << "MODE -oo (delete two oper): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE -oo (delete two oper): SUCSESS" << ENDCOLOR << std::endl;
	if (bob.getLastMsg() != ":alice!~alice@localhost MODE #alicechan -oo bob judy\r\n" 
		|| judy.getLastMsg() != ":alice!~alice@localhost MODE #alicechan -oo bob judy\r\n")
		std::cout << RED << "MODE -oo (delete two oper, broadcast): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE -oo (delete two oper, broadcast): SUCSESS" << ENDCOLOR << std::endl;
	std::cout << BLUE << "Channel mode +iklt is testing..." << ENDCOLOR << std::endl;
	/*MODE +iklt*/
	args = "MODE #alicechan +iklt key 3";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 482 * #alicechan :You're not channel operator\r\n")
		std::cout << RED << "MODE +iklt (from not an oper): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE  +iklt (from not an oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan +iklt key 3";
	server.executeCMD(args, alice);
	if (bob.getLastMsg() != ":alice!~alice@localhost MODE #alicechan +iklt key 3\r\n")
		std::cout << RED << "MODE +iklt (from an oper): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE  +iklt (from an oper): SUCSESS" << ENDCOLOR << std::endl;
	if (!server.searchChannel("#alicechan")->getInviteSettings() || !server.searchChannel("#alicechan")->getKeySetting()
		|| server.searchChannel("#alicechan")->getLimitNumeric() != 3 || !server.searchChannel("#alicechan")->getTopicRestr()
		|| server.searchChannel("#alicechan")->getKey() != "key")
		std::cout << RED << "MODE +iklt (mode appling): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE  +iklt (mode appling): SUCSESS" << ENDCOLOR << std::endl;
	//judy leave channel
	args = "PART #alicechan :bye";
	server.executeCMD(args, judy);
	if (!server.searchChannel("#alicechan")->clientIsMember(judy.getFd()))
		std::cout << GREEN << "PART (leave chan): SUCSESS" << ENDCOLOR << std::endl;
	else
		std::cout << RED << "PART (leave chan): FAIL " << ENDCOLOR << std::endl;
	if (bob.getLastMsg() != ":judy!~judy@localhost PART #alicechan :bye\r\n" 
		|| alice.getLastMsg() != ":judy!~judy@localhost PART #alicechan :bye\r\n")
		std::cout << RED << "PART (broadcast): FAIL " << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "PART (broadcast): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan -iklt";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 442 * #alicechan :You're not on that channel\r\n" )
		std::cout << RED << "MODE from not a chan member: FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "MODE from not a chan member: SUCSESS" << ENDCOLOR << std::endl;
	/*Test JOIN while restricted*/
	args = "JOIN #alicechan";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 475 * #alicechan :Cannot join channel (+k)\r\n" )
		std::cout << RED << "JOIN while +k MODE (without key): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "JOIN while +k MODE (without key): SUCSESS" << ENDCOLOR << std::endl;

	args = "JOIN #alicechan key";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 473 * #alicechan :Cannot join channel (+i)\r\n" )
		std::cout << RED << "JOIN while +i MODE (not invited): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "JOIN while +i MODE (not invited): SUCSESS" << ENDCOLOR << std::endl;

	args = "MODE #alicechan +l 2";
	server.executeCMD(args, alice);
	args = "INVITE judy #alicechan";
	server.executeCMD(args, alice);
	args = "JOIN #alicechan key";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 471 * #alicechan :Cannot join channel (+l)\r\n" )
		std::cout << RED << "JOIN while +l MODE (3d while capacity is 2): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "JOIN while +l MODE (3d while capacity is 2): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan +l 3";
	server.executeCMD(args, alice);
	args = "JOIN #alicechan key";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 366 judy #alicechan :End of /NAMES list\r\n" )
		std::cout << RED << "JOIN while +ikl MODE (all set): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "JOIN while +ikl MODE (all set): SUCSESS" << ENDCOLOR << std::endl;
	/*Set topic*/
	args = "TOPIC #alicechan :topic";
	server.executeCMD(args, judy);
	if (judy.getLastMsg() != ":server 482 * #alicechan :You're not channel operator\r\n" )
		std::cout << RED << "TOPIC while +t MODE (not a chan oper): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "TOPIC while +t MODE (not a chan oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "TOPIC #alicechan :topic";
	server.executeCMD(args, alice);
	if (alice.getLastMsg() != ":alice!~alice@localhost TOPIC #alicechan :topic\r\n"
		|| bob.getLastMsg() != ":alice!~alice@localhost TOPIC #alicechan :topic\r\n" || judy.getLastMsg() != ":alice!~alice@localhost TOPIC #alicechan :topic\r\n" )
		std::cout << RED << "TOPIC while +t MODE (chan oper): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "TOPIC while +t MODE (chan oper): SUCSESS" << ENDCOLOR << std::endl;
	args = "MODE #alicechan -t";
	server.executeCMD(args, alice);
	args = "TOPIC #alicechan :judi_topic";
	server.executeCMD(args, judy);
	if (alice.getLastMsg() != ":judy!~judy@localhost TOPIC #alicechan :judi_topic\r\n" 
		|| bob.getLastMsg() != ":judy!~judy@localhost TOPIC #alicechan :judi_topic\r\n" || judy.getLastMsg() != ":judy!~judy@localhost TOPIC #alicechan :judi_topic\r\n")
		std::cout << RED << "TOPIC whithout topic restricted MODE (not a chan oper): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "TOPIC whithout topic restricted MODE (not a chan oper): SUCSESS" << ENDCOLOR << std::endl;
	
	std::cout << "Alice: " << alice.getLastMsg();
	std::cout << "Bob: " << bob.getLastMsg();
	std::cout << "Judy: " << judy.getLastMsg();
}