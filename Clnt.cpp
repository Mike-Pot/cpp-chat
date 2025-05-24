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
	mq.mutex_.lock();		
	for (auto& mes : mq.messages_)
	{
		switch(mes.id)
		{
		case MES_QUIT:
			res = false;
			break;
		default:
			std::cout << mes.author << std::endl;
			std::cout << mes.content << std::endl;
			std::cout << std::endl;
			break;
		}
	}	
	mq.messages_.clear();
	mq.mutex_.unlock();
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
	put_quit_message(out_messages);
	
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
	client_send_message(login_);
	acc_thread.join();	

	threads.push_back(std::thread(&Client::getter, this, in_sock, std::ref(in_messages)));
	Messages mes = get_from_queue(in_messages);
	
	if (mes.back().author != "" || mes.back().content != ACK)
	{		
		logger("Login failed");
		return false;
	}
	logger("Login OK");
	client_send_message(name);
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
			client_send_message(command.cont);
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
std::string Client::make_send_string(Mes& mes)
{
	return mes.c_length + mes.content;
}

void Client::get_message(tcp::socket* sock, MessageQueue& mes)
{
	std::string author = get_author(sock);
	std::string content = get_content(sock);
	put_to_queue(in_messages, make_message(author, content));
}

void Client::client_send_message(std::string cont)
{
	put_to_queue(out_messages, make_message("", cont));
}

