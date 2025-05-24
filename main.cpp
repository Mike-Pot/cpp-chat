#include "Clnt.h"
#include "Srv.h"

int main(int argc, char* argv[])
{	
	if (argc != 2)
	{
		std::cout << "Wrong arguments number";
	}		
	else if (argv[1][0] == 'c')
	{
		Client client;
		if (client.start_client())
		{
			client.client_work();
		}
		else
		{
			std::cout << "Error starting client";
		}
	}
	else if (argv[1][0] == 's')
	{
		Server server;
		server.start_server();
	}
	else
	{
		std::cout << "no such arg";
	}
}