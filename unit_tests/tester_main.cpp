#include "Tester.hpp"

int main() {
	Server server(6667, "123");
	FakeClient alice(1, "127.0.0.1");
	FakeClient bob(2, "127.0.0.2");
	FakeClient judy(3, "127.0.0.3");

	server.addClient(alice);
	server.addClient(bob);
	server.addClient(judy);
	
	test_registration(server, alice, bob, judy);
	test_privat_msg(server, alice, bob, judy);

}