#include "Tester.hpp"

void test_registration(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy) {
	std::string args;

	std::cout << BLUE << "Registration testing..." << ENDCOLOR << std::endl;
	/*wrong password*/
	args = "PASS wrong_pass";
	server.executeCMD(args, alice);
	if (alice.getLastMsg() != ":server 464 * :Password incorrect\r\n")
		std::cout << RED << "PASS wrong: FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "PASS wrong: SUCSESS" << ENDCOLOR << std::endl;
	/*correct password*/
	args = "PASS 123";
	server.executeCMD(args, alice);
	if (alice.getHasPassword() != true)
		std::cout << RED << "PASS correct: FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "PASS correct: SUCSESS" << ENDCOLOR << std::endl;
	/*New connection without password*/
	args = "NICK bob";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 464 * :Password incorrect\r\n") {
		std::cout << bob.getLastMsg() << std::endl;
		std::cout << RED << "PASS not defined: FAIL" << ENDCOLOR << std::endl;
		}
	else
		std::cout << GREEN << "PASS not defined: SUCSESS" << ENDCOLOR << std::endl;
	args = "PASS 123";
	server.executeCMD(args, bob); //correct password for bob
	/*NICK command*/
	args = "NICK alice";
	server.executeCMD(args, alice);
	if (alice.getNickname() != "alice")
		std::cout << RED << "NICK command: FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "NICK command: SUCSESS" << ENDCOLOR << std::endl;
	/*NICK command (nick already in use)*/
	args = "NICK alice";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 433 * alice :Nickname is already in use\r\n") {
		std::cout << bob.getLastMsg() << std::endl;
		std::cout << RED << "NICK command (nickname in use): FAIL" << ENDCOLOR << std::endl;
	}
	else
		std::cout << GREEN << "NICK command (nickname in use): SUCSESS" << ENDCOLOR << std::endl;
	/*NICK command (not valide NICK)*/
	args = "NICK 1alice";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 432 * 1alice :Erroneus nickname\r\n")
		std::cout << RED << "NICK command (starts from nbr): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "NICK command (starts from nbr): SUCSESS" << ENDCOLOR << std::endl;
	args = "NICK bob#";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 432 * bob# :Erroneus nickname\r\n")
		std::cout << RED << "NICK command ('#' in nick): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "NICK command ('#' in nick): SUCSESS" << ENDCOLOR << std::endl;
	args = "NICK :bob";
	server.executeCMD(args, bob);
	if (bob.getLastMsg() != ":server 432 * :bob :Erroneus nickname\r\n")
		std::cout << RED << "NICK command (':' in nick): FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "NICK command (':' in nick): SUCSESS" << ENDCOLOR << std::endl;
	/*USER command*/
	args = "USER alice 0 * :Alice Wonderland";
	server.executeCMD(args, alice);
	if (alice.getUsername() != "alice" && alice.getRealname() != "Alice Wonderland")
		std::cout << RED << "USER command: FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "USER command: SUCSESS" << ENDCOLOR << std::endl;
	args = "USER judy 0 * :Judy Hey";
	server.executeCMD(args, alice);
	if (alice.getLastMsg() != ":server 462 * :You may not reregister\r\n")
		std::cout << RED << "USER command repeated: FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "USER command repeated: SUCSESS" << ENDCOLOR << std::endl;
	/*-----------------Alice should be registered-----------------*/
	if (alice.getRegistered() == false)
		std::cout << RED << "User registration: FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "User registration: SUCSESS" << ENDCOLOR << std::endl;
	/*-----------------Bob isn't registered-----------------*/
	if (bob.getRegistered() != false)
		std::cout << RED << "User registration without USER command: FAIL" << ENDCOLOR << std::endl;
	else
		std::cout << GREEN << "User registration without USER command: SUCSESS" << ENDCOLOR << std::endl;
	args = "NICK bob";
	server.executeCMD(args, bob);
	args = "USER bob 0 * :Bob Tween Peaks"; //Register bob
	server.executeCMD(args, bob);
	/*Register Judy*/
	args = "PASS 123";
	server.executeCMD(args, judy);
	args = "NICK judy";
	server.executeCMD(args, judy);
	args = "USER judy 0 * :Pure Evil";
	server.executeCMD(args, judy);
	if (bob.getRegistered() == false || judy.getRegistered() == false || alice.getRegistered() == false)
		std::cout << RED << "Bob and Judy weren't regestered" << ENDCOLOR << std::endl;
	else
		std::cout << PINK << "Alice, Bob and Judy are regestered" << ENDCOLOR << std::endl;

}