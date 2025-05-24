#pragma once
#include "General.h"

class Server;
class ClientHandler : public General
{
public:	
	friend class Server;
	ClientHandler(io_context& io_, MessageQueue& mq, tcp::socket* sock) 
		: io(io_), serv_in_messages(mq) {
		in_sock = sock;
	};
	~ClientHandler();
	bool start_handler();
private:
	std::string make_send_string(Mes& mes) override;;
	void get_message(tcp::socket* sock, MessageQueue& mes) override;

	void handler_send_message(std::string cont);
	bool client_login(std::string login);

	io_context& io;	
	MessageQueue out_messages;
	MessageQueue& serv_in_messages;
};

using Handlers = std::vector<ClientHandler*>;
class Server 
{
public:	
	bool start_server(ip::port_type server_port = DEF_SRV_PORT);
	~Server();
	MessageQueue in_messages;
private:
	void server_shuttle(MessageQueue& in_messages,
		Handlers& client_handlers);
	void server_acceptor(io_context& io, ip::port_type server_port);

	io_context io;
	Handlers client_handlers;	
	std::vector<std::thread> threads;
	IoHandler io_handler;
};