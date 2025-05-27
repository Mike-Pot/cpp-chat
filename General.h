#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>


#include <sdkddkver.h>
#define BOOST_DISABLE_CURRENT_LOCATION
#include "boost/asio.hpp"

using namespace boost::asio;
using tcp = ip::tcp;

#include "Constatnts.h"
static std::atomic_bool execution_stop = false;

/*user commands*/
enum class COMMANDS { QUIT, SEND, SHOW, ERR };

struct Command
{
	COMMANDS comm;
	std::string cont;
};

/*message*/
struct Mes
{
	int id;
	std::string type;
	std::string a_length; 	
	std::string author; 
	std::string c_length;
	std::string content;
};
using Messages = std::vector<Mes>;

/*message queue*/
struct MessageQueue
{
	Messages get_from_queue(); //wait for messages
	Messages drain_queue(); //take messages and return
	void put_to_queue(Mes mes);
	void put_quit_message();
private:
	Messages messages_;
	std::mutex mutex_;
	std::condition_variable event_;
};

struct OutMessageQueue : public MessageQueue
{
	void put_disconnect_message(const std::string& name);
	void put_login_message(const std::string& login,
		const std::string& content = "");
	void put_ordinary_message(const std::string& author, const std::string& content);
	void put_create_user_message(const std::string& name);
	void put_create_room_message(const std::string& name);
};

/*for debugging*/
struct Logger
{
	void operator()(std::string s);
};
static Logger  logger;

/*console io*/
struct IoHandler
{
	bool out(MessageQueue& mq);
	Command in();
	void out_string(const std::string& s);
private:
	std::mutex console_mutex_;
};

/*general functions*/
tcp::socket* connector(io_context& io, const tcp::endpoint& ep);
tcp::socket* accept_connection(io_context& io, const tcp::endpoint& ep);

Mes make_message(const std::string& type, const std::string& auth,
	const std::string& cont, int id = 0);
Mes get_message(tcp::socket* sock);
std::string get_string(tcp::socket* sock, int ch_read);

/*parse user input*/
Command command_parser(std::string s);

/*base class for client and client handler*/
class General
{
protected:
	/*thread functions*/
	void sender(tcp::socket* sock, OutMessageQueue& out_messages);
	void getter(tcp::socket* sock, MessageQueue& in_messages);

	std::string name;
	tcp::socket* in_sock = nullptr;
	tcp::socket* out_sock = nullptr;
	std::vector<std::thread> threads;
};

/*debagging function*/
static void show_mes(Mes& m)
{
	std::cout << m.id << std::endl << '/';
	std::cout << m.type << '/' << std::endl;
	std::cout << m.a_length << '/' << std::endl;
	std::cout << m.author << '/' << std::endl;
	std::cout << m.c_length << '/' << std::endl;
	std::cout << m.content << '/' << std::endl;
}



