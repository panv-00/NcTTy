/*
 * File:   NrInclude.h
 * Author: Michel Alwan
 *
 * Created on 2022-12-31
 */

#ifndef NRINCLUDE_H
#define NRINCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <thread>

#define MAX_MESSAGES     1000
#define BUFSIZE          1024
#define TIMEOUT          5

#define TAB              9
#define ESCAPE_CHAR      27
#define PAGE_UP          53
#define PAGE_DN          54
#define ARROW_CHAR       91
#define ARROW_UP         65
#define ARROW_DN         66
#define ARROW_RT         67
#define ARROW_LT         68
#define PAGE_UP_DN       126
#define BACKSPACE        127
#define START_INPUT      "\r\x1b[2C"

#define ADDRESS          "76.235.94.33"
//#define ADDRESS          "127.0.0.1"
#define PORT             6667
 
typedef enum
{
  CLR_DEFAULT  =  0,
  CLR_BLACK_FG = 30,
  CLR_RED_FG       ,
  CLR_GREEN_FG     ,
  CLR_YELLOW_FG    ,
  CLR_BLUE_FG      ,
  CLR_MAGENTA_FG   ,
  CLR_CYAN_FG      ,
  CLR_WHITE_FG     ,

  CLR_BLACK_BG = 40,
  CLR_RED_BG       ,
  CLR_GREEN_BG     ,
  CLR_YELLOW_BG    ,
  CLR_BLUE_BG      ,
  CLR_MAGENTA_BG   ,
  CLR_CYAN_BG      ,
  CLR_WHITE_BG

} ColorCode;

typedef enum
{
  NO_NET_ERROR = 0,
  SOCKET_ERROR,
  SERVER_ERROR,
  SSL_ERROR,
  ERROR_RECEIVING_DATA,
  CONNECTION_CLOSED

} NetworkStatus;

#endif
