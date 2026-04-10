#include "Tester.hpp"

int test_ping(Server& server, FakeClient& client) {
	std::string args;
	int i = 0;

	std::cout << BLUE << "PING server is testing..." << ENDCOLOR << std::endl;
	args = "PING :from alice";
	server.executeCMD(args, client);
	if (client.getLastMsg() != ":server PONG from alice\r\n") {
		std::cout << RED << "PING: FAIL" << ENDCOLOR << std::endl;
		i++;
	}
	else 
		std::cout << GREEN << "PING: SUCSESS" << ENDCOLOR << std::endl;
	return (i);
}

int main() {
	Server server(6667, "123");
	FakeClient* alice = new FakeClient(101, "127.0.0.1");
	FakeClient* bob = new FakeClient(102, "127.0.0.2");
	FakeClient* judy = new FakeClient(103, "127.0.0.3");

	server.addClient(*alice);
	server.addClient(*bob);
	server.addClient(*judy);
	
	int i = 0;
	i += test_registration(server, *alice, *bob, *judy);
	i += test_privat_msg(server, *alice, *bob, *judy);
	i += test_join(server, *alice, *bob, *judy);
	i += test_chan_logic(server, *alice, *bob, *judy);
	i += test_ping(server, *alice);
	if (i == 0)
		std::cout << PINK << "All tests PASSED!" << ENDCOLOR << std::endl;
	return (i);
}