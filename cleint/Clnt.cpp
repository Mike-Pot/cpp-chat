#include "Clnt.h"


void Client::client_acceptor(io_context& io, tcp::endpoint& ep, tcp::socket** psock)
{    
    tcp::socket* sock = accept_connection(io, ep);
    *psock = sock;  
}

Client::~Client()
{
    out_messages.put_quit_message();

    if (in_sock)
    {
        in_sock->close();
    }
    if (out_sock)
    {
        out_sock->close();
    }

    for (auto& t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    delete in_sock;
    delete out_sock;

}

bool Client::start_client(ip::port_type client_port, ip::port_type server_port,
    std::string server_addr)
{   
    out_sock = connector(io, { ip::make_address(server_addr), server_port });
    if (!out_sock)
    {      
        return false;
    }  
    threads.push_back(std::thread(&Client::sender, this, out_sock, std::ref(out_messages))); 
    tcp::endpoint ep(ip::make_address(ANY_ADDR), client_port);
    std::thread acc_thread(&Client::client_acceptor, this, std::ref(io),
        std::ref(ep), &in_sock);
    out_messages.put_login_message(login);
    acc_thread.join();   
    threads.push_back(std::thread(&Client::getter, this, in_sock));   
    return true;
}

void Client::set_handler(MessageHandler mh)
{
    message_handler = mh;
}

void Client::set_name(const std::string& name)
{
    out_messages.author = name;
}

void Client::set_token(const std::string& token)
{
    out_messages.token = token;
}

void Client::send_broadcast_message(const std::string& content)
{
    out_messages.put_ordinary_message("", content);
}

void Client::sender(tcp::socket* sock, OutMessageQueue& out_messages)
{   
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
                buffer(mes.to_string()),err);           
        }
    }    
}

void Client::getter(tcp::socket* sock)
{
    bool finish = false;   
    while (!finish)
    {
        try
        {
            Mes mes = get_message(sock);            
            message_handler(mes);
        }
        catch (...)
        {
            finish = true;
        }
    }    
}
