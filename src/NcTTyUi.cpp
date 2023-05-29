/*
 * File:   NcTTyUi.cpp
 * Created on 2023-04-17
 */

#include "NcTTyUi.h"
#include "NcTTyNet.h"

std::string wrap_string(const std::string &str, uint16_t width, int &lines)
{
  std::string wrapped_string;
  uint16_t line_start = 0;
  uint16_t line_end = width;

  while (line_end < str.length())
  {
    while (str[line_end] != ' ' && line_end > line_start)
    {
      line_end--;
    }

    wrapped_string += str.substr(line_start, line_end - line_start) + "\n\r";
    line_start = line_end + 1;
    line_end = line_start + width;
    lines++;
  }

  wrapped_string += str.substr(line_start);
  lines++;

  return wrapped_string;
}

NcTTyUi::NcTTyUi() : app_run{true}, scroll_amount{0}
{
  exit_message = "Thank you for using nctty.";
  net = new NcTTyNet();

  _EnableRawMode();

  setbuf(stdout, NULL);
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
}

NcTTyUi::~NcTTyUi()
{
  delete net;

  ClearScreen();
  _DisableRawMode();

  printf("\033[5 q");
  printf("%s\n", exit_message.c_str());
}

void NcTTyUi::ClearScreen() { printf("\033[2J\033[3J\033[1;1H"); }
void NcTTyUi::EraseInDisplay(ClearCode code) { printf("\033[%dJ", int(code)); }
void NcTTyUi::EraseInLine(ClearCode code) { printf("\033[%dK", int(code)); }
void NcTTyUi::MoveTo(int row, int col) { printf("\033[%d;%dH", row, col); }
void NcTTyUi::MoveUp(int n) { printf("\033[%dA", n); }
void NcTTyUi::MoveDn(int n) { printf("\033[%dB", n); }
void NcTTyUi::MoveRt(int n) { printf("\033[%dC", n); }
void NcTTyUi::MoveLt(int n) { printf("\033[%dD", n); }
void NcTTyUi::SaveLocation() { printf("\0337"); }
void NcTTyUi::RestoreLocation() { printf("\0338"); }
void NcTTyUi::SetColor(ColorCode code) { printf("\x1b[%dm", code); }

void NcTTyUi::SetupApp()
{
  _SetInsertMode(true);
  ClearScreen();
  MoveTo(2, 4);
  printf("Welcome to NakenChat for TTY.");
  MoveTo(4, 2);
  printf("Select Username");
  EraseInLine(CURSOR_TO_END);
  _PrintPrompt();
  ReadInput();
  _Trim(message);
  username = message;

  MoveTo(4, 2);
  printf("Select Password [empty for none]");
  _PrintPrompt();
  EraseInLine(CURSOR_TO_END);
  ReadInput();
  _Trim(message);
  password = message;
  printf("\r\nconnecting %s with password '%s'...", username.c_str(),
         password.c_str());

  NetworkStatus connect_status = net->Connect();

  switch (connect_status)
  {
  case NO_NET_ERROR:
  {
    connected = true;
    message_buffer = "";
    ClearScreen();
    _PrintPrompt();

    std::string connect_string;

    if (password.length() > 0)
    {
      connect_string = ".n " + username + "=" + password;
    }

    else
    {
      connect_string = ".n " + username;
    }

    std::string connect_message = "\% Connected using nctty...";

    SSL_write(net->GetSSL(), connect_string.c_str(),
              connect_string.length() + 1);
    SSL_write(net->GetSSL(), connect_message.c_str(),
              connect_message.length() + 1);
    std::thread receive_thread([&]() { net->ReceiveData(this); });
    receive_thread.detach();
  }
  break;

  default:
  {
    exit_message = NetStatusToText(connect_status);
    app_run = false;
  }
  break;
  }
}

void NcTTyUi::Run()
{
  std::string command_message;

  while (app_run)
  {
    _SetInsertMode(true);
    ReadInput();
    _Trim(message);

    if (!strcmp(message, ".q"))
    {
      if (connected)
      {
        ClearScreen();
        MoveTo(1, 1);
        printf("Closing Connection!\r\n");
        net->StopReceiving();
        while (!net->FinishedReceiving())
        {
          sleep(1);
        }
      }

      app_run = false;
    }

    else
    {
      if (connected)
      {
        SSL_write(net->GetSSL(), message, message_length + 1);
      }
      else
      {
        exit_message = "Disconnected from server!";
        app_run = false;
      }
    }
  }
}

void NcTTyUi::ReadInput()
{
  int c;
  message_length = 0;
  message[0] = 0;
  cursor_index = 0;
  cursor_col = 4;

  while (true)
  {
    c = 0;
    int res = read(STDIN_FILENO, &c, 1);

    if (res == -1)
    {
      exit(1);
    }

    else if (res == 0)
    {
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
      usleep(10);
      continue;
    }

    // insert mode
    if (insert_mode)
    {
      if (c == CTRL_KEY('c'))
      {
        _SetInsertMode(false);
      }

      if (c == '\r' || c == '\n')
      {
        _PrintPrompt();
        return;
      }

      else if (c == BACKSPACE)
      {
        _DeleteChar();
      }

      else if (c == ESCAPE_CHAR)
      {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
        {
          continue;
        }

        if (read(STDIN_FILENO, &seq[1], 1) != 1)
        {
          continue;
        }

        if (seq[0] == ARROW_CHAR)
        {
          if (seq[1] == ARROW_UP)
          {
            _ScrollUp();
          }

          else if (seq[1] == ARROW_DN)
          {
            _ScrollDown();
          }

          else if (seq[1] == ARROW_RT)
          {
            _MoveCursorRight();
          }

          else if (seq[1] == ARROW_LT)
          {
            _MoveCursorLeft();
          }
        }
      }

      else
      {
        if (c > 31 && c < 127)
        {
          _InsertChar(c);
        }
      }
    }

    // cursor mode
    else
    {
      if (c == 'i')
      {
        _SetInsertMode(true);
      }

      else if (c == 'h')
      {
        _MoveCursorLeft();
      }

      else if (c == 'l')
      {
        _MoveCursorRight();
      }

      else if (c == 'j')
      {
        _ScrollDown();
      }

      else if (c == 'k')
      {
        _ScrollUp();
      }

      else if (c == '\r' || c == '\n')
      {
        _PrintPrompt();
        return;
      }
    }
  }
}

void NcTTyUi::AddBufferToMessages(const char *buffer, int bytes_received)
{
  std::string message_string = "";

  message_buffer.append(buffer, bytes_received);

  for (int i = 0; i < bytes_received; i++)
  {
    if (buffer[i] == '\0' || buffer[i] == '\r')
    {
      continue;
    }

    if (buffer[i] == '\n')
    {
      message_string = message_string.substr(0, message_string.find('\r'));
      message_string += '\0';
      _AddMessageToMessages(_StringToMessage(message_string));
      message_buffer.erase(0, i + 1);
      message_string = "";
    }

    else
    {
      message_string += buffer[i];
    }
  }
}
////////////////////////////////////////////////////////////////////////////////

void NcTTyUi::_DisableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldt); }

void NcTTyUi::_EnableRawMode()
{
  tcgetattr(STDIN_FILENO, &oldt);

  newt = oldt;
  newt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  newt.c_oflag &= ~(OPOST);
  newt.c_cflag |= (CS8);
  newt.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  newt.c_cc[VMIN] = 0;
  newt.c_cc[VTIME] = 1;

  newt.c_cc[14] = 1; // disable cursor blinking

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &newt);
}

void NcTTyUi::_SetInsertMode(bool mode)
{
  insert_mode = mode;
  printf("\033[%d q", mode ? 5 : 1);
  SaveLocation();
  MoveTo(w.ws_row - 1, 1);
  putchar(insert_mode ? 'I' : 'C');
  RestoreLocation();
}

void NcTTyUi::_MoveCursorRight()
{
  if (cursor_index < message_length)
  {
    cursor_index++;

    if (cursor_col < (w.ws_col - 1) - 1)
    {
      cursor_col++;
      MoveRt(1);
    }

    else
    {
      _PrintPrompt();
      for (int i = cursor_index - (w.ws_col - 1) + 5; i < cursor_index; i++)
      {
        putchar(message[i]);
      }
    }
  }
}

void NcTTyUi::_MoveCursorLeft()
{
  if (cursor_index > 0)
  {
    cursor_index--;

    if (cursor_col > 4)
    {
      cursor_col--;
      MoveLt(1);
    }

    else
    {
      SaveLocation();

      for (int i = cursor_index; i < cursor_index + (w.ws_col - 1) - 5; i++)
      {
        putchar(message[i]);
      }

      RestoreLocation();
    }
  }
}

void NcTTyUi::_ScrollUp()
{
  if (!first_message_visible)
  {
    scroll_amount--;
    _UpdateDisplay();
  }
}

void NcTTyUi::_ScrollDown()
{
  if (scroll_amount < 0)
  {
    scroll_amount++;
    _UpdateDisplay();
  }
}

void NcTTyUi::_DeleteChar()
{
  if (cursor_index > 0 && message_length > 0)
  {
    memmove(&message[cursor_index - 1], &message[cursor_index],
            message_length - cursor_index + 1);
    message_length--;
    cursor_index--;
    printf("\b");

    for (int i = cursor_index; i < message_length; i++)
    {
      printf("%c", message[i]);
    }

    printf(" ");

    for (int i = cursor_index; i <= message_length; i++)
    {
      printf("\b");
    }
  }
  message[message_length] = 0;
}

void NcTTyUi::_InsertChar(char c)
{
  if (message_length == MSG_MAX_LENGTH - 1)
  {
    return;
  }

  if (cursor_index == message_length)
  {
    message[message_length++] = c;
  }

  else
  {
    memmove(&message[cursor_index + 1], &message[cursor_index],
            message_length - cursor_index);

    message[cursor_index] = c;
    message_length++;
  }

  message[message_length] = 0;
  cursor_index++;

  if (cursor_col < (w.ws_col - 1) - 1)
  {
    putchar(c);
    cursor_col++;
  }

  else
  {
    _PrintPrompt();

    for (int i = cursor_index - (w.ws_col - 1) + 5; i < cursor_index; i++)
    {
      putchar(message[i]);
    }
  }

  SaveLocation();

  for (int i = cursor_index; i < (w.ws_col - 1) - 5; i++)
  {
    if (i < message_length)
    {
      putchar(message[i]);
    }
  }

  RestoreLocation();
}

void NcTTyUi::_PrintPrompt(int row, int col)
{
  MoveTo(row, col);
  printf("\033[0J > ");
}

void NcTTyUi::_PrintPrompt() { _PrintPrompt(w.ws_row - 1, 2); }

void NcTTyUi::_ClearDisplay()
{
  MoveTo(w.ws_row - 1, 1);
  EraseInDisplay(CURSOR_TO_START);
}

void NcTTyUi::_Trim(char *str)
{
  char *start = str;
  char *end = str + strlen(str) - 1;

  while (isspace((unsigned char)*start))
  {
    start++;
  }

  while (end > start && isspace((unsigned char)*end))
  {
    end--;
  }

  *(end + 1) = '\0';

  if (start != str)
  {
    memmove(str, start, (end - start) + 2);
  }
}

void NcTTyUi::_AddMessageToMessages(Message msg)
{
  if (all_messages.size() == BUFSIZE)
  {
    all_messages.erase(all_messages.begin());
  }

  all_messages.push_back(msg);

  if (scroll_amount == 0)
  {
    _UpdateDisplay();
  }
}

Message NcTTyUi::_StringToMessage(std::string message_string)
{
  size_t start_uname, end_uname;
  size_t start_from, end_from, start_to, end_to, start_body;
  Message msg;

  if (message_string[0] == '<')
  {
    msg.type = MSG_PRIVATE_RECEIVED;

    start_from = message_string.find("<");
    end_from = message_string.find(" (private):");
    msg.from = message_string.substr(start_from, end_from - start_from);

    msg.to = "";

    start_body = end_from + 12;
    msg.body = message_string.substr(start_body);
  }

  else if (message_string.compare(0, 18, ">> Message sent to") == 0)
  {
    msg.type = MSG_PRIVATE_SENT;

    msg.from = "";

    start_to = message_string.find("[");
    end_to = message_string.find(":");
    msg.to = message_string.substr(start_to, end_to - start_to);

    start_body = message_string.find(" (private):") + 12;
    msg.body = message_string.substr(start_body);
  }

  else if (message_string[0] == '(')
  {
    msg.type = MSG_EMOTE;
    msg.from = "";
    msg.to = "";
    msg.body = message_string;
  }

  else if (message_string[0] == '[')
  {
    start_uname = message_string.find("]") + 1;
    end_uname = message_string.find(":");
    std::string uname =
        message_string.substr(start_uname, end_uname - start_uname);

    if (uname == username)
    {
      msg.type = MSG_SENT;
    }
    else
    {
      msg.type = MSG_RECEIVED;
    }

    start_from = message_string.find("[");
    end_from = message_string.find(":");
    msg.from = message_string.substr(start_from, end_from - start_from);

    msg.to = "";

    start_body = end_from + 2;
    msg.body = message_string.substr(start_body);
  }

  else if (message_string[0] == '>' && message_string[1] == '>')
  {
    msg.type = MSG_SYSTEM;
    msg.from = "";
    msg.to = "";
    msg.body = message_string;
  }

  else
  {
    msg.type = MSG_NONE;
    msg.from = "";
    msg.to = "";
    msg.body = message_string;
  }

  return msg;
}

int NcTTyUi::_PrintMessage(Message msg, bool emulate)
{

  std::string formatted;

  SetColor(CLR_DEFAULT);

  switch (msg.type)
  {
  case MSG_NONE:
  {
    formatted = msg.body;
    SetColor(CLR_DEFAULT);
  }
  break;

  case MSG_SENT:
  {
    formatted = msg.from + ": " + msg.body;
    SetColor(CLR_BLUE_FG);
  }
  break;

  case MSG_RECEIVED:
  {
    formatted = msg.from + ": " + msg.body;
    SetColor(CLR_DEFAULT);
  }
  break;

  case MSG_PRIVATE_SENT:
  {
    formatted = "-> " + msg.to + ": " + msg.body;
    SetColor(CLR_MAGENTA_FG);
  }
  break;

  case MSG_PRIVATE_RECEIVED:
  {
    formatted = "<- " + msg.from + ": " + msg.body;
    SetColor(CLR_MAGENTA_BG);
    SetColor(CLR_BLACK_FG);
  }
  break;

  case MSG_EMOTE:
  {
    formatted = msg.body;
    SetColor(CLR_GREEN_BG);
    SetColor(CLR_BLACK_FG);
  }
  break;

  case MSG_SYSTEM:
  {
    formatted = msg.body;
    SetColor(CLR_RED_FG);
  }
  break;
  }

  formatted = formatted.substr(0, formatted.find('\0'));

  int return_value = 0;

  if (emulate)
  {
    wrap_string(formatted, w.ws_col, return_value);
  }

  else
  {
    printf("%s\r\n", wrap_string(formatted, w.ws_col, return_value).c_str());
  }

  SetColor(CLR_DEFAULT);

  return return_value;
}

void NcTTyUi::_UpdateDisplay()
{
  // ClearScreen();
  SaveLocation();
  _ClearDisplay();
  MoveTo(1, 1);

  int total_lines = 0;
  start_index = 0;

  for (int i = all_messages.size() - 1; i >= 0; i--)
  {
    total_lines += _PrintMessage(all_messages[i], true);

    if (total_lines >= w.ws_row - 2)
    {
      break;
    }

    start_index = i;
  }

  total_lines = 0;

  for (size_t i = start_index + scroll_amount; i < all_messages.size(); i++)
  {
    total_lines += _PrintMessage(all_messages[i], true);
    if (total_lines < w.ws_row - 2)
    {
      _PrintMessage(all_messages[i], false);
    }
  }

  first_message_visible = (start_index + scroll_amount == 0);
  RestoreLocation();
}
