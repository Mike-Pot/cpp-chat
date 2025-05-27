#include "General.h"

void Logger::operator()(std::string s)
{
	/*uncomment this string to turn logger on*/
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

 Mes get_message(tcp::socket* sock)
{
	std::string type = get_string(sock, T_LEN);
	std::string author = get_string(sock, std::stoi(get_string(sock, A_LEN)));
	std::string content = get_string(sock, std::stoi(get_string(sock, C_LEN)));
	return make_message(type, author, content);
}
 
std::string get_string(tcp::socket* sock, int ch_read)
{
	char buf[MAX_BUF];	
	read(*sock, buffer(buf), transfer_exactly(ch_read));	
	return std::string(buf,ch_read);
}

void MessageQueue::put_to_queue(Mes mes)
{
	std::unique_lock<std::mutex> lk(mutex_);	
	messages_.push_back(mes);
	lk.unlock();
	event_.notify_all();
}

Messages MessageQueue::get_from_queue()
{	
	std::unique_lock<std::mutex> lk(mutex_);
	event_.wait(lk, [&] {
		return (messages_.size() > 0);
		});
	Messages res(messages_.begin(), messages_.end());
	messages_.clear();
	lk.unlock();
	return res;
}

Messages MessageQueue::drain_queue()
{
	mutex_.lock();
	Messages res(messages_.begin(), messages_.end());
	messages_.clear();
	mutex_.unlock();
	return res;
}

void MessageQueue::put_quit_message()
{
	put_to_queue(make_message(QUIT, "", ""));
}

void OutMessageQueue::put_disconnect_message(const std::string& name)
{	
	put_to_queue(make_message(DISCONNECT, name, ""));
}

void OutMessageQueue::put_login_message(const std::string& login, const std::string& content)
{
	put_to_queue(make_message(LOGIN, login, content));
}

void OutMessageQueue::put_ordinary_message(const std::string& author, const std::string& content)
{
	put_to_queue(make_message(ORDINARY, author, content));
}

void OutMessageQueue::put_create_user_message(const std::string& name)
{
	put_to_queue(make_message(CREATE, USER, name));
}

void OutMessageQueue::put_create_room_message(const std::string& name)
{
	put_to_queue(make_message(CREATE, ROOM, name));
}

Mes make_message(const std::string& type, const std::string& auth,
	const std::string& cont, int id)
{
	std::string sz_a = std::to_string(auth.size());
	sz_a.resize(A_LEN, ' ');
	std::string sz_c = std::to_string(cont.size());
	sz_c.resize(C_LEN, ' ');	
	return { id,type,sz_a,auth,sz_c,cont };
}

void General::sender(tcp::socket* sock, OutMessageQueue& out_messages)
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
				buffer(mes.type + mes.a_length + mes.author + mes.c_length + mes.content),
				err);
			if (err)
			{
				logger("Error sending message");
			}
		}
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
			Mes mes = get_message(sock);
			in_messages.put_to_queue(mes);
		}
		catch (...)
		{		
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
