# cpp-chat
Это - каркас, на который можно будет навесить собственно функционал. Сейчас важно оценить(и по возможности потестировать!) общую структуру и логику сетевого взаимодействия. 
# структура 
Единое приложение, запускается в серверном или клинетском варианте аргументом командной строки. Сервер и клиент имеют локальные адреса (127.0.0.1), порты, логины, имена клиентов - по умолчанию. Авторизуются все. Приложение принимает сообщение от клиентов и рассылает их всем остальным.
# логика
Сервер:

Создает челночный поток и поток-акцептор, последний слушает произвольный адрес, при подключении создает объект-обработчик. Обработчик подключается к клиенту, авторизует его, создает два рабочих потока - на вход и на выход. Входной ждет на своем сокете и складывает сообщения в очередь сервера, выходной - ждет на очереди обработчика, забирает оттуда сообщения и пишет их в сокет. Челночный поток ждет на очереди сервера, забирает оттуда сообщения и раскладывает их по очередям обработчиков. Основной поток крутится в цикле ввода с консоли, получив команду выхода, завершает все потоки и удаляет обработчики.

Клиент:

Пытется подключится с серверу(который должен быть запушен), при успехе посылает логин-сообщение и создает два потока, как обработчик сервера. Входной складывает сообщения во входную очередь. Основной поток забирает их оттуда по команде "отобразить"
и выводит в консоль, сам в цикле принимает консольный ввод и складывает в выходную очередь, на которой ждет выходной поток. По команде выхода выходит из цикла и завершает работу.
# запуск и работа
 Clnt -c клиент
 
 Clnt -s сервер
 
 Команды:
 
 -q выход
 
 -s отобразить сообщения (только клиент)
 

 
