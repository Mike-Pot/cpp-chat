/*
//Клиент реализован в виде объекта, создаваемого основным приложением. Логин, имя, токен, порты и ip для подключения хранятся во внтуренних переменных
//Подключение
Client client; //логин по умолчанию ИЛИ
Client client(string login);
Client.set_handler(...); //передаем обработчик сообщений, как в minimal.cpp
bool success = client.start_client(); //пытается подключиться к серверу и отправть ему логин-сообщение, успех если удалось
//Сервер отвечает NAK если в подключении отказано, ACK и токен в параметре - если принято
client.set_token(token);
client.set_name(name);
//Интерфейс 
set_name(string name) //устанавливает имя 
set_token(string token) //устанавливает токен 
send_***_message(string...)//отправвляет в чат сообщение, в зависимости от его типа различаются параметры; пока метод один
broadcast(string content) //сообщение для всех
//В момент прихода сообщения от сервера вызывается переданный при создании обработчик основного приложения, он получает структуру
struct Mes
{
    int id; //пока 0
    std::string type; // прописаны с General.h
    std::string token; //5 символов сейчас
    std::string a_length;
    std::string author; // от кого сообщение
    std::string n_length; 
    std::string name; // кому или в какую комнату
    std::string c_length; 
    std::string content; // собственно текст
    //в ответах сервера на управляющие сообщения три последних поля - параметры и интерпретируются по ситуации
}
 */
