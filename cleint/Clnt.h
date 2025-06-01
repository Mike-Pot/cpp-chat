#pragma once

#include "General.h"

using MessageHandler = std::function<void(const Mes&)>;

class Client
{
public:

    Client(std::string login_ = DEF_LOGIN)
        : login(login_) {};
    ~Client();
    bool start_client(ip::port_type client_port = DEF_CL_PORT, ip::port_type server_port = DEF_SRV_PORT,
        std::string server_addr = LOC_ADDR);  
    void set_handler(MessageHandler mh);   
    void set_name(const std::string& name);
    void set_token(const std::string& token);
    void send_broadcast_message(const std::string& content);
private:
    void sender(tcp::socket* sock, OutMessageQueue& out_messages);
    void getter(tcp::socket* socks);    
    void client_acceptor(io_context& io, tcp::endpoint& ep, tcp::socket** psock);
   
    OutMessageQueue out_messages;    
    std::string login;
    io_context io;    
    tcp::socket* in_sock = nullptr;
    tcp::socket* out_sock = nullptr;
    std::vector<std::thread> threads;
    MessageHandler message_handler;   
};


