#include "Clnt.h"

void Client::client_acceptor(io_context& io, tcp::endpoint& ep, tcp::socket** psock)
{
	logger("Client acceptor started");
	tcp::socket* sock = accept_connection(io, ep);
	*psock = sock;
	logger("Client acceptor finished");
}

bool IoHandler::out(MessageQueue& mq)
{
	bool res = true;
	console_mutex_.lock();
	Messages messages = mq.drain_queue();
	for (auto& mes : messages)
	{
		if (mes.type == QUIT)
		{
			res = false;
			continue;
		}		
		std::cout << mes.author << std::endl;
		std::cout << mes.content << std::endl;
		std::cout << std::endl;	
	}	
	console_mutex_.unlock();
	return res;
}

Command IoHandler::in()
{
	std::string s;
	console_mutex_.lock();
	std::getline(std::cin, s);
	console_mutex_.unlock();
	return command_parser(s);
}

void IoHandler::out_string(const std::string& s)
{
	console_mutex_.lock();
	std::cout << s << std::endl;
	console_mutex_.unlock();
}

Client::~Client()
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

bool Client::start_client(ip::port_type client_port, ip::port_type server_port,
	std::string server_addr)
{
	logger("Client started");	

	out_sock = connector(io, { ip::make_address(server_addr), server_port });
	if (!out_sock)
	{
		logger("Error connecting to server");
		return false;
	}
	logger("Connected to server");

	threads.push_back(std::thread(&Client::sender, this, out_sock, std::ref(out_messages)));

	tcp::endpoint ep(ip::make_address(ANY_ADDR), client_port);
	std::thread acc_thread(&Client::client_acceptor, this, std::ref(io),
		std::ref(ep), &in_sock);
	out_messages.put_login_message(login_);
	acc_thread.join();	

	threads.push_back(std::thread(&Client::getter, this, in_sock, std::ref(in_messages)));
	Messages mes = in_messages.get_from_queue();
	if(!check_login(mes.back(), login_))
	{		
		logger("Login failed");
		return false;
	}
	logger("Login OK");
	out_messages.put_create_user_message(name);	
	return true;
}

void Client::client_work()
{	
	bool finish = false;	
	while (!finish)
	{
		Command command = io_handler.in();
		switch (command.comm)
		{
		case COMMANDS::QUIT:
			finish = true;
			break;
		case COMMANDS::SEND:
			out_messages.put_ordinary_message(name, command.cont);
			break;
		case COMMANDS::SHOW:
			finish = !io_handler.out(in_messages);
			break;
		case COMMANDS::ERR:
			io_handler.out_string("Wrong command");			
			break;
		default:
			break;
		}
	}
}

bool Client::check_login(Mes& mes, const std::string login)
{
	bool res = false;
	if (mes.type == LOGIN && mes.author == login && mes.content == ACK)
	{
		res = true;
	}
	return res;
}