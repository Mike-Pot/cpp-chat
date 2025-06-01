#include "General.h"

tcp::socket* connector(io_context& io, const tcp::endpoint& ep)
{
    tcp::socket* sock = new tcp::socket(io);
    boost::system::error_code err;   
    sock->connect(ep, err);
    return err ? nullptr : sock;
}

tcp::socket* accept_connection(io_context& io, const tcp::endpoint& ep)
{
    tcp::socket* sock = new tcp::socket(io);
    tcp::acceptor acc(io, ep);    
    acc.accept(*sock);    
    return sock;
}

Mes get_message(tcp::socket* sock)
{
    std::string type = get_string(sock, TP_LEN);
    std::string token = get_string(sock, TK_LEN);
    std::string author = get_string(sock, std::stoi(get_string(sock, A_LEN)));
    std::string name = get_string(sock, std::stoi(get_string(sock, N_LEN)));
    std::string content = get_string(sock, std::stoi(get_string(sock, C_LEN)));
    return make_message(type, token, author, name, content);
}

std::string get_string(tcp::socket* sock, int ch_read)
{
    char buf[MAX_BUF];
    read(*sock, buffer(buf), transfer_exactly(ch_read));
    return std::string(buf, ch_read);
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
    put_to_queue(make_message(QUIT));
}

void OutMessageQueue::put_ordinary_message(const std::string& name,
    const std::string& content)
{
    put_to_queue(make_message(ORDINARY, token, author, name, content));
}

void OutMessageQueue::put_login_message(const std::string& login, const std::string& pass)
{
    put_to_queue(make_message(LOGIN, token, login, pass));
}

void OutMessageQueue::put_acknowledge_message(const std::string& param1, 
    const std::string& param2, const std::string& param3)
{
    put_to_queue(make_message(ACK, token, param1, param2, param3));
}

Mes make_message(
    const std::string& type,
    const std::string& token,
    const std::string& author,
    const std::string& name,
    const std::string& cont,
    int id)
{
    std::string sz_a = std::to_string(author.size());
    sz_a.resize(A_LEN, ' ');
    std::string sz_n = std::to_string(name.size());
    sz_n.resize(N_LEN, ' ');
    std::string sz_c = std::to_string(cont.size());
    sz_c.resize(C_LEN, ' ');
    return { id,type,token,sz_a,author,sz_n,name,sz_c,cont };
}


