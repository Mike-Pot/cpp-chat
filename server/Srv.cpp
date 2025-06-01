#include "Srv.h"

bool Server::start_server(ip::port_type server_port)
{
	logger("Server started");
	threads.push_back(std::thread(&Server::server_acceptor, this, std::ref(io),
		server_port));
	threads.push_back(std::thread(&Server::server_shuttle, this, std::ref(in_messages),
		std::ref(client_handlers)));
	std::string s;
	bool finish = false;
	while (!finish)
	{
		std::getline(std::cin, s);
		Command command = command_parser(s);
		switch (command.comm)
		{
		case COMMANDS::QUIT:
			finish = true;
			execution_stop = true;
			connector(io, { ip::make_address(LOC_ADDR),DEF_SRV_PORT });
			break;
		case COMMANDS::ERR:
			logger("Wrong command");
			break;
		default:			
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
		Mes mes = get_message(in_sock);		
		out_messages.put_acknowledge_message();
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

void ClientHandler::sender(tcp::socket* sock, OutMessageQueue& out_messages)
{
	logger("Sender thread started");
	boost::system::error_code err;
	bool finished = false;
	while (!finished)
	{
		Messages messages = out_messages.get_from_queue();
		for (auto& mes : messages)
		{
			if (mes.type == QUIT)
			{
				finished = true;
			}
			write(*sock,
				buffer(mes.to_string()),err);			
		}
	}
	logger("Sender thread finished");
}

void ClientHandler::getter(tcp::socket* sock, MessageQueue& in_messages)
{
	bool finish = false;
	logger("Getter thread started");
	while (!finish)
	{
		try
		{
			Mes mes = get_message(sock);
			in_messages.put_to_queue(mes);
		}
		catch (...)
		{
			logger("Error getting message");
			finish = true;
		}
	}
	logger("Getter thread finished");
}

Command command_parser(std::string s)
{
	Command res = { COMMANDS::ERR };
	if (s == COMM_QUIT)
	{
		res = { COMMANDS::QUIT };
	}
	return res;
}

void Logger::operator()(const std::string& s)
{
	m_.lock();
	std::cout << s << std::endl;
	m_.unlock();
}

std::string Logger::get()
{
	std::string res;
	m_.lock();
	std::getline(std::cin, res);
	m_.unlock();
	return res;
}
