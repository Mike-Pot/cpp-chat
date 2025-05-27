#include "Srv.h"

bool Server::start_server(ip::port_type server_port)
{
	logger("Server started");	
	threads.push_back(std::thread(&Server::server_acceptor, this, std::ref(io),
			server_port));
	threads.push_back(std::thread(&Server::server_shuttle, this, std::ref(in_messages),
		std::ref(client_handlers)));
	bool finish = false;
	while (!finish)
	{
		Command command = io_handler.in();
		switch (command.comm)
		{
		case COMMANDS::QUIT:
			finish = true;
			execution_stop = true;
			connector(io, { ip::make_address(LOC_ADDR),DEF_SRV_PORT });
			break;		
		case COMMANDS::ERR:
			io_handler.out_string("Wrong command");
			break;
		default:
			io_handler.out_string("Not a command");
			break;
		}
	}
	return true;
}

Server::~Server()
{	
	in_messages.put_quit_message();
	for (auto ch : client_handlers)
	{
		delete ch;
	}
	for (auto& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
}

void Server::server_shuttle(MessageQueue& in_messages, Handlers& client_handlers)
{
	logger("Shuuttle started");	
	bool finished = false;
	while (!finished)
	{
		Messages messages = in_messages.get_from_queue();
		for (auto& mes : messages)
		{
			if (mes.type == QUIT)
			{
				finished = true;
			}
			for (auto c_h : client_handlers)
			{	
				c_h->out_messages.put_to_queue(mes);
			}
		}
	}
	logger("Shuttle finished");
}

void Server::server_acceptor(io_context& io, ip::port_type server_port)
{
	logger("Acceptor started");
	while (true)
	{	
		tcp::socket* sock = accept_connection(io, { ip::make_address(ANY_ADDR),server_port });
		if (execution_stop)
		{
			break;
		}
		ClientHandler* c_h = new ClientHandler(io, in_messages, sock);
		if (c_h->start_handler())
		{
			client_handlers.push_back(c_h);
		}
		else
		{
			delete c_h;
		}
	}
	logger("Acceptor stopped");
}

ClientHandler::~ClientHandler()
{
	out_messages.put_quit_message();
	
	if (in_sock)
	{
		in_sock->close();
	}
	if (out_sock)
	{
		out_sock->close();
	}	
	for (auto& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	delete in_sock;
	delete out_sock;
}

bool ClientHandler::start_handler()
{
	logger("Handler started");
	out_sock = connector(io, { ip::make_address(LOC_ADDR),DEF_CL_PORT });
	if (!out_sock)
	{
		logger("Error connecting to client");
		return false;	
	}
	logger("Client connected");
	threads.push_back(std::thread(&ClientHandler::sender, this, out_sock, std::ref(out_messages)));
	try
	{
		std::string login = get_message(in_sock).author;		
		out_messages.put_login_message(login, ACK);
		name = get_message(in_sock).content;
	}
	catch (...)
	{
		logger("Error logging client");		
		out_messages.put_quit_message();
		return false;
	}
	logger("Client login OK");
	threads.push_back(std::thread(&ClientHandler::getter, this, in_sock, 
		std::ref(serv_in_messages)));
	return true;
}

bool ClientHandler::client_login(std::string login)
{
	return true;
}
