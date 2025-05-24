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

static std::atomic_bool execution_stop = false;

const std::string LOC_ADDR = "127.0.0.1";
const std::string ANY_ADDR = "0.0.0.0";
const ip::port_type DEF_CL_PORT = 2002;
const ip::port_type DEF_SRV_PORT = 2001;
 
const int A_LEN = 2;
const int C_LEN = 5;
const int MAX_BUF = 512;

const int MES_QUIT = -1;
const int MES_DISCONNECT = -2;

const std::string ACK = "ACK";
const std::string NAK = "NAK";

enum class COMMANDS { QUIT, SEND, SHOW, ERR };
const char COMM_SIGN = '/';
const std::string COMM_QUIT = "q";
const std::string COMM_SHOW = "s";
struct Command
{
	COMMANDS comm;
	std::string cont;
};

struct Mes
{
	int id;	
	std::string a_length; 	
	std::string author; 
	std::string c_length;
	std::string content;
};
using Messages = std::vector<Mes>;

struct MessageQueue
{
	std::mutex mutex_;
	std::condition_variable event_;
	Messages messages_;
};

struct Logger
{
	void operator()(std::string s);
};
static Logger  logger;

struct IoHandler
{
	bool out(MessageQueue& mq);
	Command in();
	void out_string(const std::string& s);
private:
	std::mutex console_mutex_;
};

tcp::socket* connector(io_context& io, const tcp::endpoint& ep);
tcp::socket* accept_connection(io_context& io, const tcp::endpoint& ep);

void put_to_queue(MessageQueue& mq, Mes mes);
Messages get_from_queue(MessageQueue& mq);
void put_quit_message(MessageQueue& mq);
void put_disconnect_message(MessageQueue& mq, std::string& name);
Mes make_message(const std::string& auth, const std::string& cont, int id = 0);

std::string get_string(tcp::socket* sock, int ch_read);
std::string get_author(tcp::socket* sock);
std::string get_content(tcp::socket* sock);

Command command_parser(std::string s);
class General
{
protected:
	void sender(tcp::socket* sock, MessageQueue& out_messages);
	void getter(tcp::socket* sock, MessageQueue& in_messages);

	virtual std::string make_send_string(Mes& mes) = 0;
	virtual void get_message(tcp::socket* sock, MessageQueue& mes) = 0;

	std::string name;
	tcp::socket* in_sock = nullptr;;
	tcp::socket* out_sock = nullptr;
	std::vector<std::thread> threads;
};
static void show_mes(Mes& m)
{
	std::cout << m.id << std::endl;
	std::cout << m.a_length << '/' << std::endl;
	std::cout << m.author << '/' << std::endl;
	std::cout << m.c_length << '/' << std::endl;
	std::cout << m.content << '/' << std::endl;
}



