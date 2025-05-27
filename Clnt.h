#pragma once
#include "General.h"

const std::string DEF_LOGIN = "login";
const std::string DEF_NAME = "anonimous";

class Client : public General
{
public:
	Client(std::string name_ = DEF_NAME, std::string login = DEF_LOGIN)
		: login_(login) {
		name = name_;
	};
	~Client();
	bool start_client(ip::port_type client_port = DEF_CL_PORT, ip::port_type server_port = DEF_SRV_PORT,
		std::string server_addr = LOC_ADDR);
	void client_work();
private:
	bool check_login(Mes& mes, const std::string login);
	void client_acceptor(io_context& io, tcp::endpoint& ep, tcp::socket** psock);

	MessageQueue in_messages;
	OutMessageQueue out_messages;	
	IoHandler io_handler;	
	std::string login_;
	io_context io;
};


