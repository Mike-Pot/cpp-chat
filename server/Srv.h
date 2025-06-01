#pragma once
#include "General.h"

static std::atomic_bool execution_stop = false;

struct Logger
{
	void operator()(const std::string& s);
	std::string get();
private:
	std::mutex m_;
};
static Logger logger;

const std::string COMM_QUIT = "q";

enum class COMMANDS { QUIT, ERR };

struct Command
{
	COMMANDS comm;
	std::vector<std::string> params;
};
Command command_parser(std::string s);

class Server;
class ClientHandler 
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
	bool client_login(std::string login); //for now unused
	void sender(tcp::socket* sock, OutMessageQueue& out_messages);
	void getter(tcp::socket* sock, MessageQueue& in_messages);

	std::string name;
	tcp::socket* in_sock = nullptr;
	tcp::socket* out_sock = nullptr;
	std::vector<std::thread> threads;

	io_context& io;
	OutMessageQueue out_messages;
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
};

static void show_message(Mes& mes)
{
	std::cout << mes.type << std::endl;
	std::cout << mes.token << std::endl;
	std::cout << mes.a_length << std::endl;
	std::cout << mes.author << std::endl;
	std::cout << mes.n_length << std::endl;
	std::cout << mes.name << std::endl;
	std::cout << mes.c_length << std::endl;
	std::cout << mes.content << std::endl;
}