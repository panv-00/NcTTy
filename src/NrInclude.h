/*
 * File:   NrInclude.h
 * Author: Michel Alwan
 *
 * Created on 2023-01-30
 */

#ifndef NRINCLUDE_H
#define NRINCLUDE_H

#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <signal.h>
#include <thread>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>

#define VERSION "0.0.2"

#define ADDRESS          "76.235.94.33"
//#define ADDRESS          "127.0.0.1"
#define PORT             6667
 
#define PROMPT_HEIGHT 4
#define BUFSIZE    2048

#define TAB          9
#define ESCAPE_CHAR 27
#define PAGE_UP     53
#define PAGE_DN     54
#define ARROW_CHAR  91
#define ARROW_UP    65
#define ARROW_DN    66
#define ARROW_RT    67
#define ARROW_LT    68
#define PAGE_UP_DN 126
#define BACKSPACE  127

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
  MSG_NONE = 0,
  MSG_SENT,
  MSG_RECEIVED,
  MSG_PRIVATE_SENT,
  MSG_PRIVATE_RECEIVED,
  MSG_EMOTE,
  MSG_SYSTEM

} MessageType;

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
