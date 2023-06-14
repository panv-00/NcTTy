#ifndef NCTTYUI_H
#define NCTTYUI_H

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

#include "NcMessage.h"

#define CTRL_KEY(k) ((k)&0x1f)

#define MAX_PROMPT 512
#define SMALL_STRING 17
#define LARGE_STRING 4096
#define MAX_MESSAGES 1024

#define MIN_TERM_WIDTH 55
#define MIN_TERM_HEIGHT 14

#define NEW_LINE "\r\n"
#define CLEAR_TO_END "\x1b[K"
#define HIDE_CURSOR "\x1b[?25l"
#define SHOW_CURSOR "\x1b[?25h"
#define CURSOR_INSERT "\x1b[5 q"
#define CURSOR_MOVE "\x1b[1 q"
#define HOME_POSITION "\x1b[H"

#define CLR_DEFAULT "\x1b[0m"
#define CLR_BLACK_FG "\x1b[30m"
#define CLR_RED_FG "\x1b[31m"
#define CLR_GREEN_FG "\x1b[32m"
#define CLR_YELLOW_FG "\x1b[33m"
#define CLR_BLUE_FG "\x1b[34m"
#define CLR_MAGENTA_FG "\x1b[35m"
#define CLR_CYAN_FG "\x1b[36m"
#define CLR_WHITE_FG "\x1b[37m"
#define CLR_BLACK_BG "\x1b[40m"
#define CLR_RED_BG "\x1b[41m"
#define CLR_GREEN_BG "\x1b[42m"
#define CLR_YELLOW_BG "\x1b[43m"
#define CLR_BLUE_BG "\x1b[44m"
#define CLR_MAGENTA_BG "\x1b[45m"
#define CLR_CYAN_BG "\x1b[46m"
#define CLR_WHITE_BG "\x1b[47m"

#define TAB 9
#define ENTER 13
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

typedef struct
{
  unsigned int cur_x;
  unsigned int cur_y;
  unsigned int screen_rows;
  unsigned int screen_cols;
  char *screen_buffer;
  size_t buffer_size;
  size_t buffer_length;
  struct termios orig_term;

} Terminal;

class NcTTyNet;

class NcTTyUi
{
public:
  NcTTyUi();
  ~NcTTyUi();

  void Run();
  void AddBufferToMessages(const char *buffer, int bytes_received);

  void EndConnection() { connected = false; };

private:
  void _DisableRawMode();
  void _EnableRawMode();
  bool _GetScreenSize(unsigned int *rows, unsigned int *cols);
  void _SetupScreenBuffer(bool resize_signal);
  bool _HasPureChars(const char *s);
  unsigned int _PrAppendSB(const char *s, unsigned int length);
  unsigned int _PrAppendSB(const char *s, unsigned int length, bool simulate);
  unsigned int _AppendSB(const char *s, unsigned int length);
  void _PromptToSB();
  void _QuitApp();
  void _ForceQuit();
  void _RunPrompt();
  void _DeleteChar();
  void _ScrollUp();
  void _ScrollDown();
  void _MoveCursorRight();
  void _MoveCursorLeft();
  void _InsertChar(char c);
  char _Getch();
  void _GetCharacter();
  void _DisplayTermWidthWarning();
  void _DisplayWelcome();
  void _DisplayMessages();
  void _DisplayConnecting();
  void _DisplayDisconnecting();
  void _PrintPrompt();
  void _RefreshScreen();
  void _AddMessageToMessages(NcMessage *msg);
  size_t _Trim(char *str);
  void _Fatal(const char *s);
  void _CleanUp();

  Terminal term;

  bool app_has_error;
  bool in_raw_mode;
  bool in_insert_mode;
  bool accept_input;
  bool app_is_running;
  bool get_username;
  bool get_password;
  bool connected;
  bool disconnecting;

  char username[SMALL_STRING + 1];
  char password[SMALL_STRING + 1];
  char *prompt;
  size_t prompt_cursor_index;
  size_t prompt_length;

  char message_buffer[LARGE_STRING + 1];
  size_t message_buffer_length;
  NcMessage all_messages[MAX_MESSAGES];
  size_t number_of_messages;
  size_t first_message_index;
  int scroll_amount;

  NcTTyNet *net;
};

#endif
