/*
 * File:   NcTTyUi.h
 * Created on 2023-04-17
 */

#ifndef NCTTYUI_H
#define NCTTYUI_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define MSG_MAX_LENGTH 512
#define BUFSIZE 2048

#define CTRL_KEY(k) ((k)&0x1f)

#define TAB 9
#define ESCAPE_CHAR 27
#define PAGE_UP 53
#define PAGE_DN 54
#define ARROW_CHAR 91
#define ARROW_UP 65
#define ARROW_DN 66
#define ARROW_RT 67
#define ARROW_LT 68
#define PAGE_UP_DN 126
#define BACKSPACE 127

typedef enum
{
  CLR_DEFAULT = 0,
  CLR_BLACK_FG = 30,
  CLR_RED_FG,
  CLR_GREEN_FG,
  CLR_YELLOW_FG,
  CLR_BLUE_FG,
  CLR_MAGENTA_FG,
  CLR_CYAN_FG,
  CLR_WHITE_FG,

  CLR_BLACK_BG = 40,
  CLR_RED_BG,
  CLR_GREEN_BG,
  CLR_YELLOW_BG,
  CLR_BLUE_BG,
  CLR_MAGENTA_BG,
  CLR_CYAN_BG,
  CLR_WHITE_BG

} ColorCode;

typedef enum
{
  CURSOR_TO_END = 0,
  CURSOR_TO_START,
  START_TO_END

} ClearCode;

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

class NcTTyNet;

typedef struct
{
  MessageType type;
  std::string from;
  std::string to;
  std::string body;

} Message;

class NcTTyUi
{
public:
  NcTTyUi();
  ~NcTTyUi();

  void ClearScreen();
  void EraseInDisplay(ClearCode code);
  void EraseInLine(ClearCode code);
  void MoveTo(int row, int col);
  void MoveUp(int n);
  void MoveDn(int n);
  void MoveRt(int n);
  void MoveLt(int n);
  void SaveLocation();
  void RestoreLocation();
  void SetColor(ColorCode code);

  void SetupApp();
  void Run();
  void ReadInput();

  void AddBufferToMessages(const char *buffer, int bytes_received);
  void EndConnection() { connected = false; };

private:
  void _DisableRawMode();
  void _EnableRawMode();
  void _SetInsertMode(bool mode);
  void _MoveCursorRight();
  void _MoveCursorLeft();
  void _ScrollUp();
  void _ScrollDown();
  void _DeleteChar();
  void _InsertChar(char c);
  void _PrintPrompt(int row, int col);
  void _PrintPrompt();
  void _ClearDisplay();

  void _Trim(char *str);

  void _AddMessageToMessages(Message msg);
  Message _StringToMessage(std::string message_string);
  int _PrintMessage(Message msg, bool emulate);
  void _UpdateDisplay();

  struct winsize w;
  struct termios oldt, newt;
  int cursor_index;
  int cursor_col;
  int message_length;
  char message[MSG_MAX_LENGTH];

  bool app_run;
  bool insert_mode;

  std::string username;
  std::string password;

  bool connected;
  NcTTyNet *net;

  std::vector<Message> all_messages;
  size_t start_index;
  int scroll_amount;
  bool first_message_visible;
  std::string message_buffer;
  std::string exit_message;
};

#endif
