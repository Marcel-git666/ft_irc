#include "Tester.hpp"

void test_join(Server &server, FakeClient &alice, FakeClient &bob, FakeClient &judy) {
	std::string args;

	args = "JOIN #newchan";
	server.executeCMD(args, alice);
	if (alice.getLastMsg() != ":alice!alice@localhost JOIN #mycah\r\n");
}