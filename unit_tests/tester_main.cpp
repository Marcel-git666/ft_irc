#include "../inc/Tester.hpp"

int main() {
	Server server(6667, "123");
	FakeClient alice(1, "127.0.0.1");
	FakeClient bob(2, "127.0.0.2");

	server.addClient(alice);
	server.addClient(bob);
	
	
	test_registration(server, alice, bob);


}