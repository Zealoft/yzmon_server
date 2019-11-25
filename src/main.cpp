#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>

#include "package.hpp"
#include "Server.hpp"


Server *server;

void quit_action(int sig)
{
	delete server;
	exit(EXIT_SUCCESS);
}

int main()
{
	daemon(1, 1);
	signal(SIGINT, quit_action);
	signal(SIGTERM, quit_action);
	server = new Server;
	server->Process_Connections();
	return 0;
}