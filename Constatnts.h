#pragma once

/*connection defaults*/
const std::string LOC_ADDR = "127.0.0.1";
const std::string ANY_ADDR = "0.0.0.0";
const ip::port_type DEF_CL_PORT = 2002;
const ip::port_type DEF_SRV_PORT = 2001;

/*message field lengths*/
const int T_LEN = 3;
const int A_LEN = 2;
const int C_LEN = 5;

/*message buffer size*/
const int MAX_BUF = 512;
 
/*message types*/
const std::string ORDINARY = "ORD";
const std::string CREATE = "CRT";
const std::string LOGIN = "LOG";
const std::string DISCONNECT = "DSC";
const std::string QUIT = "MQT";


/*create types*/
const std::string USER = "user";
const std::string ROOM = "room";

/*reply types*/
const std::string ACK = "ACK";
const std::string NAK = "NAK";

/*commands*/
const char COMM_SIGN = '/';
const std::string COMM_QUIT = "q";
const std::string COMM_SHOW = "s";
