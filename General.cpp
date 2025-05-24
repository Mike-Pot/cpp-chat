#include "General.h"

void Logger::operator()(std::string s)
{
	//std::cout << s << std::endl;
	return;
}

tcp::socket* connector(io_context& io, const tcp::endpoint& ep)
{
	tcp::socket* sock = new tcp::socket(io);
	boost::system::error_code err;
	logger("Connecting...");
	sock->connect(ep, err);	
	return err ? nullptr : sock;
}

tcp::socket* accept_connection(io_context& io, const tcp::endpoint& ep)
{
	tcp::socket* sock = new tcp::socket(io);
	tcp::acceptor acc(io, ep);
	logger("Accepting connection...");	
	acc.accept(*sock);	
	logger("Connection accepted");	
	return sock;
}

std::string get_author(tcp::socket* sock)
{	
	int len = std::stoi(get_string(sock, A_LEN));	
	return get_string(sock, len);
}

std::string get_content(tcp::socket* sock)
{	
	int len = std::stoi(get_string(sock, C_LEN));	
	return get_string(sock, len);
}

std::string get_string(tcp::socket* sock, int ch_read)
{
	char buf[MAX_BUF];	
	read(*sock, buffer(buf), transfer_exactly(ch_read));	
	return std::string(buf,ch_read);
}

void put_to_queue(MessageQueue& mq, Mes mes)
{
	std::unique_lock<std::mutex> lk(mq.mutex_);
	mq.messages_.push_back(mes);
	lk.unlock();
	mq.event_.notify_all();
}

Messages get_from_queue(MessageQueue& mq)
{
	std::unique_lock<std::mutex> lk(mq.mutex_);
	mq.event_.wait(lk, [&] {
		return (mq.messages_.size() > 0);
		});	
	Messages res(mq.messages_.begin(), mq.messages_.end());
	mq.messages_.clear();	
	lk.unlock();
	return res;
}
void put_quit_message(MessageQueue& mq)
{
	put_to_queue(mq, make_message("", "", MES_QUIT));
}
void put_disconnect_message(MessageQueue& mq, std::string& name)
{
	put_to_queue(mq, make_message(name, "", MES_DISCONNECT));
}
Mes make_message(const std::string& auth, const std::string& cont, int id)
{
	std::string sz_a = std::to_string(auth.size());
	sz_a.resize(A_LEN, ' ');
	std::string sz_c = std::to_string(cont.size());
	sz_c.resize(C_LEN, ' ');	
	return { id,sz_a,auth,sz_c,cont };
}

void General::sender(tcp::socket* sock, MessageQueue& out_messages)
{
	logger("Sender thread started");
	boost::system::error_code err;
	bool finished = false;
	std::unique_lock<std::mutex> lk(out_messages.mutex_);
	while (!finished)
	{
		out_messages.event_.wait(lk, [&] {
			return (out_messages.messages_.size() > 0);
			});	
		for (auto& mes : out_messages.messages_)
		{
			if (mes.id == MES_QUIT || mes.id == MES_DISCONNECT)
			{
				finished = true;
			}			
			write(*sock, buffer(make_send_string(mes)), err);
			if (err)
			{
				logger("Error sending message");				
			}			
		}
		out_messages.messages_.clear();
	}	
	logger("Sender thread finished");
}

void General::getter(tcp::socket* sock, MessageQueue& in_messages)
{
	bool finish = false;
	logger("Getter thread started");
	while (!finish)
	{
		try
		{
			get_message(sock, in_messages);			
		}
		catch (...)
		{
			put_disconnect_message(in_messages, name); 			
			finish = true;
		}
	}
	logger("Getter thread finished");
}

Command command_parser(std::string s)
{
	Command res = { COMMANDS::ERR,"" };
	if (s[0] == COMM_SIGN)
	{
		s = s.substr(1);
		if (s == COMM_QUIT)
		{
			res = { COMMANDS::QUIT,"" };
		}
		if (s == COMM_SHOW)
		{
			res = { COMMANDS::SHOW,"" };
		}
	}
	else
	{
		res = { COMMANDS::SEND,s };
	}
	return res;
}
