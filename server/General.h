#pragma once

#include <sdkddkver.h>
#define BOOST_DISABLE_CURRENT_LOCATION
#include "boost/asio.hpp"

using namespace boost::asio;
using tcp = ip::tcp;

#include "Constants.h"

/*message*/
struct Mes
{
    int id;
    std::string type;
    std::string token;
    std::string a_length;
    std::string author;
    std::string n_length;
    std::string name;    
    std::string c_length;
    std::string content;
    std::string to_string() {
        return type + token + a_length + author + n_length + name + c_length + content;
    }
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
    void put_ordinary_message(const std::string& name, const std::string& content);
    void put_login_message(const std::string& login, const std::string& pass = "");
    void put_acknowledge_message(const std::string& param1 = "",
        const std::string& param2 = "", const std::string& param3 = "");
    std::string author;
    std::string token = DEF_TOK;
};

/*general functions*/
tcp::socket* connector(io_context& io, const tcp::endpoint& ep);
tcp::socket* accept_connection(io_context& io, const tcp::endpoint& ep);

Mes make_message(
    const std::string& type,
    const std::string& token = "",
    const std::string& author = "",
    const std::string& name = "",
    const std::string& cont = "",
    int id = 0);
Mes get_message(tcp::socket* sock);
std::string get_string(tcp::socket* sock, int ch_read);


