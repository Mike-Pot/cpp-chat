#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>

/*connection defaults*/
const std::string LOC_ADDR = "127.0.0.1";
const std::string ANY_ADDR = "0.0.0.0";
const ip::port_type DEF_CL_PORT = 2002;
const ip::port_type DEF_SRV_PORT = 2001;

/*message field lengths*/
const int TP_LEN = 3;
const int TK_LEN = 5;
const int A_LEN = 2;
const int N_LEN = 2;
const int C_LEN = 3;

/*message buffer size*/
const int MAX_BUF = 512;

/*message types*/
const std::string ORDINARY = "ORD";
const std::string QUIT = "MQT";
const std::string LOGIN = "LOG";

/*reply types*/
const std::string ACK = "ACK";
const std::string NAK = "NAK";

/*other defaults*/
const std::string DEF_TOK = "12345";
const std::string DEF_LOGIN = "login";
