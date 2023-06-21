#include "NcTTyUi.h"
#include "NcTTyNet.h"

NcTTyUi::NcTTyUi()
{
  net = new NcTTyNet();
  prompt = (char *)malloc((MAX_PROMPT + 1) * sizeof(char));

  if (prompt == NULL)
  {
    _Fatal("MemoryAllocation");
  }

  prompt[0] = '\0';
  prompt_length = 0;
  prompt_cursor_index = 0;

  message_buffer[0] = '\0';
  message_buffer_length = 0;

  number_of_messages = 0;
  first_message_index = 0;
  scroll_amount = 0;

  term.screen_cols = 0;
  term.screen_rows = 0;

  if (!_GetScreenSize(&term.screen_rows, &term.screen_cols))
  {
    _Fatal("GetScreenSize");
  }

  _SetupScreenBuffer(false);

  app_has_error = false;
  in_raw_mode = false;
  in_insert_mode = true;
  accept_input = true;
  app_is_running = true;
  get_username = true;
  get_password = false;
  connected = false;
  disconnecting = false;

  _EnableRawMode();

  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[3J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

NcTTyUi::~NcTTyUi()
{
  if (in_raw_mode)
  {
    _DisableRawMode();
  }

  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[3J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  if (app_has_error)
  {
    printf("    Process Terminated!\n");
  }

  else
  {
    printf("    Thank you for using nctty!\n");
  }

  _CleanUp();
}

void NcTTyUi::Run()
{
  while (app_is_running)
  {
    _RefreshScreen();
    _GetCharacter();
  }
}

void NcTTyUi::AddBufferToMessages(const char *buffer, int bytes_received)
{
  if (bytes_received <= 0)
  {
    return;
  }

  message_buffer[0] = '\0';
  message_buffer_length = 0;

  for (int i = 0; i < bytes_received; i++)
  {
    if (buffer[i] == '\0' || buffer[i] == '\r')
    {
      continue;
    }

    if (buffer[i] == '\n')
    {
      NcMessage msg;

      message_buffer[message_buffer_length] = '\0';
      msg.Parse(message_buffer, message_buffer_length, username);
      _AddMessageToMessages(&msg);
      message_buffer[0] = '\0';
      message_buffer_length = 0;
    }

    else
    {
      message_buffer[message_buffer_length++] = buffer[i];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void NcTTyUi::_DisableRawMode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term.orig_term) == -1)
  {
    _Fatal("DisableRawMode");
  }
}

void NcTTyUi::_EnableRawMode()
{
  if (tcgetattr(STDIN_FILENO, &term.orig_term) == -1)
  {
    _Fatal("EnableRawMode | PASS 1");
  }

  struct termios raw = term.orig_term;

  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
  {
    _Fatal("EnableRawMode | PASS 2");
  }

  in_raw_mode = true;
}

bool NcTTyUi::_GetScreenSize(unsigned int *rows, unsigned int *cols)
{
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
  {
    return false;
  }

  *cols = ws.ws_col;
  *rows = ws.ws_row;

  return true;
}

void NcTTyUi::_SetupScreenBuffer(bool resize_signal)
{
  if (resize_signal)
  {
    bool refresh = false;

    if (term.buffer_size != term.screen_rows * (term.screen_cols + 15))
    {
      refresh = true;
    }

    if (term.buffer_size < term.screen_rows * (term.screen_cols + 15))
    {
      term.buffer_size = term.screen_rows * (term.screen_cols + 15);
      term.screen_buffer = (char *)realloc(
          term.screen_buffer, (term.buffer_size + 1) * sizeof(char));

      if (term.screen_buffer == NULL)
      {
        _Fatal("MemoryAllocation");
      }

      term.screen_buffer[0] = '\0';
      term.buffer_length = 0;
    }

    term.cur_y = term.screen_rows;

    if (refresh)
    {
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[3J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      _RefreshScreen();
    }

    return;
  }

  term.buffer_size = term.screen_rows * (term.screen_cols + 15);
  term.screen_buffer = (char *)malloc((term.buffer_size + 1) * sizeof(char));

  if (term.screen_buffer == NULL)
  {
    _Fatal("MemoryAllocation");
  }

  term.screen_buffer[0] = '\0';
  term.buffer_length = 0;

  term.cur_x = 4;
  term.cur_y = term.screen_rows;
}

bool NcTTyUi::_HasPureChars(const char *s)
{
  if (!strcmp(s, NEW_LINE))
    return false;
  if (!strcmp(s, CLEAR_TO_END))
    return false;
  if (!strcmp(s, HIDE_CURSOR))
    return false;
  if (!strcmp(s, SHOW_CURSOR))
    return false;
  if (!strcmp(s, HOME_POSITION))
    return false;
  if (!strcmp(s, CLR_DEFAULT))
    return false;
  if (!strcmp(s, CLR_BLACK_FG))
    return false;
  if (!strcmp(s, CLR_RED_FG))
    return false;
  if (!strcmp(s, CLR_GREEN_FG))
    return false;
  if (!strcmp(s, CLR_YELLOW_FG))
    return false;
  if (!strcmp(s, CLR_BLUE_FG))
    return false;
  if (!strcmp(s, CLR_MAGENTA_FG))
    return false;
  if (!strcmp(s, CLR_CYAN_FG))
    return false;
  if (!strcmp(s, CLR_WHITE_FG))
    return false;
  if (!strcmp(s, CLR_BLACK_BG))
    return false;
  if (!strcmp(s, CLR_RED_BG))
    return false;
  if (!strcmp(s, CLR_GREEN_BG))
    return false;
  if (!strcmp(s, CLR_YELLOW_BG))
    return false;
  if (!strcmp(s, CLR_BLUE_BG))
    return false;
  if (!strcmp(s, CLR_MAGENTA_BG))
    return false;
  if (!strcmp(s, CLR_CYAN_BG))
    return false;
  if (!strcmp(s, CLR_WHITE_BG))
    return false;

  return true;
}

unsigned int NcTTyUi::_PrAppendSB(const char *s, unsigned int length)
{
  bool simulate = false;

  return _PrAppendSB(s, length, simulate);
}

unsigned int NcTTyUi::_PrAppendSB(const char *s, unsigned int length,
                                  bool simulate)
{
  unsigned int temp = 1;
  unsigned int line_start = 0;
  unsigned int line_end = term.screen_cols;
  unsigned int return_value = 0;

  while (line_start < length)
  {
    if (line_end >= length)
    {
      line_end = length;
    }
    else
    {
      int last_space = -1;
      for (unsigned int i = line_start + term.screen_cols - 1; i >= line_start;
           i--)
      {
        if (s[i] == ' ')
        {
          last_space = i;
          break;
        }
      }

      if (last_space != -1 && last_space > (int)line_start)
      {
        line_end = last_space;
      }
      else
      {
        line_end = line_start + term.screen_cols;
      }
    }

    unsigned int line_length = line_end - line_start;

    if (!simulate)
    {
      char ss[line_length + 1];
      memcpy(ss, s + line_start, line_length);
      ss[line_length] = '\0';

      temp = 1;
      temp *= _AppendSB(ss, line_length);
      temp *= _AppendSB(CLEAR_TO_END, 3);
      temp *= _AppendSB(NEW_LINE, 2);

      return_value += temp;
    }

    else
    {
      return_value++;
    }

    line_start = line_end;
    line_end += term.screen_cols;
  }

  return return_value;
}

unsigned int NcTTyUi::_AppendSB(const char *s, unsigned int length)
{
  if (term.buffer_length + length > term.buffer_size)
  {
    return 0;
  }

  for (size_t i = term.buffer_length; i < term.buffer_length + length; i++)
  {
    term.screen_buffer[i] = s[i - term.buffer_length];
  }

  term.buffer_length += length;
  term.screen_buffer[term.buffer_length] = '\0';

  return 1;
}

void NcTTyUi::_PromptToSB()
{
  unsigned int length = term.screen_cols - 8;
  unsigned int start = 0;

  if (length > prompt_length)
  {
    length = prompt_length;
  }

  if (prompt_cursor_index > length)
  {
    start = prompt_cursor_index - length;
  }

  if (term.buffer_length + length > term.buffer_size)
  {
    return;
  }

  for (size_t i = term.buffer_length; i < term.buffer_length + length; i++)
  {
    term.screen_buffer[i] = prompt[i - term.buffer_length + start];
  }

  term.buffer_length += length;
  term.screen_buffer[term.buffer_length] = '\0';
}

void NcTTyUi::_QuitApp()
{
  disconnecting = true;
  _RefreshScreen();

  if (connected)
  {
    net->StopReceiving();

    while (!net->FinishedReceiving())
    {
      sleep(1);
    }
  }

  app_is_running = false;
  disconnecting = false;
}

void NcTTyUi::_ForceQuit()
{
  app_is_running = false;
  app_has_error = true;
}

void NcTTyUi::_RunPrompt()
{
  scroll_amount = 0;
  prompt_length -= _Trim(prompt);

  if (get_username)
  {
    size_t bytes_to_copy = prompt_length;
    const char *equal_sign_pos = strchr(prompt, '=');

    if (equal_sign_pos != NULL)
    {
      bytes_to_copy = equal_sign_pos - prompt;
    }

    if (bytes_to_copy > SMALL_STRING)
    {
      bytes_to_copy = SMALL_STRING;
    }

    strncpy(username, prompt, bytes_to_copy);
    username[bytes_to_copy] = '\0';

    get_username = false;
    get_password = true;
  }

  else if (get_password)
  {
    size_t bytes_to_copy = prompt_length;

    if (bytes_to_copy > SMALL_STRING)
    {
      bytes_to_copy = SMALL_STRING;
    }

    strncpy(password, prompt, bytes_to_copy);
    password[bytes_to_copy] = '\0';

    get_password = false;
  }

  else if (!strcmp(prompt, ".q"))
  {
    _QuitApp();
  }

  else if (connected)
  {
    SSL_write(net->GetSSL(), prompt, prompt_length + 1);

    if (strncmp(prompt, ".n", 2) == 0)
    {
      char *modified_prompt = prompt + 2;
      size_t modified_prompt_length = prompt_length - 2;

      modified_prompt_length -= _Trim(modified_prompt);

      size_t bytes_to_copy = modified_prompt_length;
      const char *equal_sign_pos = strchr(modified_prompt, '=');

      if (equal_sign_pos != NULL)
      {
        bytes_to_copy = equal_sign_pos - modified_prompt;
      }

      if (bytes_to_copy > SMALL_STRING)
      {
        bytes_to_copy = SMALL_STRING;
      }

      strncpy(username, modified_prompt, bytes_to_copy);
      username[bytes_to_copy] = '\0';
    }
  }

  prompt[0] = '\0';
  prompt_length = 0;
  prompt_cursor_index = 0;
  term.cur_x = 4;
}

void NcTTyUi::_DeleteChar()
{
  if (prompt_cursor_index == 0 || prompt_length == 0)
  {
    return;
  }

  if (prompt_cursor_index != prompt_length)
  {
    memmove(&prompt[prompt_cursor_index - 1], &prompt[prompt_cursor_index],
            prompt_length - prompt_cursor_index + 1);
  }

  if (prompt_length <= term.screen_cols - 8)
  {
    term.cur_x--;
  }

  prompt_length--;
  prompt_cursor_index--;
  prompt[prompt_length] = 0;
}

void NcTTyUi::_ScrollUp()
{
  if (first_message_index + scroll_amount > 0)
  {
    scroll_amount--;
  }
}

void NcTTyUi::_ScrollDown() { scroll_amount = 0; }

void NcTTyUi::_MoveCursorRight()
{
  if (prompt_cursor_index < prompt_length)
  {
    prompt_cursor_index++;

    if (term.cur_x < term.screen_cols - 4)
    {
      term.cur_x++;
    }
  }
}

void NcTTyUi::_MoveCursorLeft()
{
  if (prompt_cursor_index == 0)
  {
    return;
  }

  prompt_cursor_index--;

  if (term.cur_x > 4 && prompt_cursor_index < term.screen_cols - 8)
  {
    term.cur_x--;
  }
}

void NcTTyUi::_MoveCursorFirst()
{
  prompt_cursor_index = 0;
  term.cur_x = 4;
}

void NcTTyUi::_MoveCursorLast()
{
  prompt_cursor_index = prompt_length;

  if (prompt_length > term.screen_cols - 8)
  {
    term.cur_x = term.screen_cols - 4;
  }

  else
  {
    term.cur_x = 4 + prompt_length;
  }
}

void NcTTyUi::_InsertChar(char c)
{
  if (prompt_length == MAX_PROMPT)
  {
    return;
  }

  if (prompt_cursor_index == prompt_length)
  {
    prompt[prompt_length++] = c;
  }

  else
  {
    memmove(&prompt[prompt_cursor_index + 1], &prompt[prompt_cursor_index],
            prompt_length - prompt_cursor_index);

    prompt[prompt_cursor_index] = c;
    prompt_length++;
  }

  prompt[prompt_length] = '\0';
  prompt_cursor_index++;

  if (term.cur_x < term.screen_cols - 4)
  {
    term.cur_x++;
  }
}

char NcTTyUi::_Getch()
{
  int result;
  char c;

  while ((result = read(STDIN_FILENO, &c, 1)) != 1)
  {
    if (result == -1 && errno != EAGAIN)
    {
      _Fatal("Getch");
    }

    else if (result == 0)
    {
      if (!_GetScreenSize(&term.screen_rows, &term.screen_cols))
      {
        _Fatal("GetScreenSize");
      }

      _SetupScreenBuffer(true);

      usleep(10);
      continue;
    }
  }

  return c;
}

void NcTTyUi::_GetCharacter()
{
  char c = _Getch();

  if (term.screen_rows < MIN_TERM_HEIGHT || term.screen_cols < MIN_TERM_WIDTH)
  {
    return;
  }

  // insert mode
  if (in_insert_mode)
  {
    if (c == CTRL_KEY('c'))
    {
      in_insert_mode = false;
    }

    if (c == CTRL_KEY('q'))
    {
      _ForceQuit();
    }

    else if (c == ENTER)
    {
      _RunPrompt();
    }

    else if (c == BACKSPACE)
    {
      _DeleteChar();
    }

    else if (c == ESCAPE_CHAR)
    {
      char seq[3];

      if ((seq[0] = _Getch()) == ARROW_CHAR)
      {
        if ((seq[1] = _Getch()) == ARROW_UP)
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
      if (accept_input && c > 31 && c < 127)
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
      in_insert_mode = true;
    }

    else if (c == 'k')
    {
      _ScrollUp();
    }

    else if (c == 'j')
    {
      _ScrollDown();
    }

    else if (c == 'l')
    {
      _MoveCursorRight();
    }

    else if (c == 'h')
    {
      _MoveCursorLeft();
    }

    else if (c == 'q')
    {
      _QuitApp();
    }

    else if (c == '0')
    {
      _MoveCursorFirst();
    }

    else if (c == '$')
    {
      _MoveCursorLast();
    }
  }
}

void NcTTyUi::_DisplayTermWidthWarning()
{
  unsigned int num_of_lines = 0;
  unsigned int num_of_chars;
  char buf[32];

  num_of_chars = (term.screen_cols < 20) ? term.screen_cols : 20;
  num_of_lines += _PrAppendSB("Terminal Size Error!", num_of_chars);

  num_of_chars = (term.screen_cols < 24) ? term.screen_cols : 24;
  num_of_lines += _PrAppendSB("Please resize terminal..", num_of_chars);

  if (term.screen_cols < MIN_TERM_WIDTH)
  {
    snprintf(buf, sizeof(buf), "  Width: %d < %d", term.screen_cols,
             MIN_TERM_WIDTH);

    num_of_chars =
        (term.screen_cols < strlen(buf)) ? term.screen_cols : strlen(buf);
    num_of_lines += _PrAppendSB(buf, num_of_chars);
  }

  if (term.screen_rows < MIN_TERM_HEIGHT)
  {
    snprintf(buf, sizeof(buf), "  Height: %d < %d", term.screen_rows,
             MIN_TERM_HEIGHT);

    num_of_chars =
        (term.screen_cols < strlen(buf)) ? term.screen_cols : strlen(buf);
    num_of_lines += _PrAppendSB(buf, num_of_chars);
  }

  for (unsigned int i = num_of_lines; i < term.screen_rows - 1; i++)
  {
    _PrAppendSB(" ", 1);
  }
}

void NcTTyUi::_DisplayWelcome()
{
  unsigned int num_of_lines = 0;

  num_of_lines += _PrAppendSB("    Welcome to nctty v1.0.1", 27);
  num_of_lines += _PrAppendSB(" Enter your username..", 22);

  if (get_password)
  {
    num_of_lines += _PrAppendSB(" Enter your password [enter for none]..", 39);
  }

  for (unsigned int i = num_of_lines; i < term.screen_rows - 1; i++)
  {
    _PrAppendSB(" ", 1);
  }
}

void NcTTyUi::_DisplayMessages()
{
  char buf[23 + SMALL_STRING + LARGE_STRING];
  size_t buffer_length = 0;
  unsigned int num_of_lines = 0;
  MessageType t;
  NcMessage *msg;

  // let's simulate at the beginning and try to get
  // the first_message_index

  for (size_t message_index = 0; message_index < number_of_messages;
       message_index++)
  {
    msg = &all_messages[number_of_messages - message_index - 1];
    t = msg->GetType();

    if (t == MSG_NONE)
    {
      if (strlen(msg->GetBody()) > term.screen_cols)
      {
        strncpy(buf, msg->GetBody(), term.screen_cols);
        buf[term.screen_cols] = '\0';
        buffer_length = term.screen_cols;
      }

      else
      {
        buffer_length = snprintf(buf, sizeof(buf), "%s", msg->GetBody());

        if (buffer_length >= sizeof(buf))
        {
          buffer_length = sizeof(buf) - 1;
        }
      }
    }

    else if (t == MSG_EMOTE)
    {
      buffer_length = snprintf(buf, sizeof(buf), "%s", msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_SYSTEM)
    {
      buffer_length = snprintf(buf, sizeof(buf), "%s", msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_SENT)
    {
      buffer_length =
          snprintf(buf, sizeof(buf), "%s: %s", msg->GetFrom(), msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_RECEIVED)
    {
      buffer_length =
          snprintf(buf, sizeof(buf), "%s: %s", msg->GetFrom(), msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_PRIVATE_SENT)
    {
      buffer_length =
          snprintf(buf, sizeof(buf), "-> %s: %s", msg->GetTo(), msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_PRIVATE_RECEIVED)
    {
      buffer_length = snprintf(buf, sizeof(buf), "<- %s: %s", msg->GetFrom(),
                               msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    num_of_lines += _PrAppendSB(buf, buffer_length, true);

    if (num_of_lines > term.screen_rows - 3)
    {
      first_message_index = number_of_messages - message_index - 1;
      break;
    }
  }

  // End of simulation.
  // Now we do the screen update

  num_of_lines = 0;

  for (size_t message_index = first_message_index + scroll_amount;
       message_index < number_of_messages; message_index++)
  {
    msg = &all_messages[message_index];
    t = msg->GetType();

    if (t == MSG_NONE)
    {
      _AppendSB(CLR_DEFAULT, 4);

      if (strlen(msg->GetBody()) > term.screen_cols)
      {
        strncpy(buf, msg->GetBody(), term.screen_cols);
        buf[term.screen_cols] = '\0';
        buffer_length = term.screen_cols;
      }

      else
      {
        buffer_length = snprintf(buf, sizeof(buf), "%s", msg->GetBody());

        if (buffer_length >= sizeof(buf))
        {
          buffer_length = sizeof(buf) - 1;
        }
      }
    }

    else if (t == MSG_EMOTE)
    {
      _AppendSB(CLR_GREEN_BG, 5);
      _AppendSB(CLR_BLACK_FG, 5);
      buffer_length = snprintf(buf, sizeof(buf), "%s", msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_SYSTEM)
    {
      _AppendSB(CLR_RED_FG, 5);
      buffer_length = snprintf(buf, sizeof(buf), "%s", msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_SENT)
    {
      _AppendSB(CLR_BLUE_FG, 5);
      buffer_length =
          snprintf(buf, sizeof(buf), "%s: %s", msg->GetFrom(), msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_RECEIVED)
    {
      _AppendSB(CLR_DEFAULT, 4);
      buffer_length =
          snprintf(buf, sizeof(buf), "%s: %s", msg->GetFrom(), msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_PRIVATE_SENT)
    {
      _AppendSB(CLR_MAGENTA_FG, 5);
      buffer_length =
          snprintf(buf, sizeof(buf), "-> %s: %s", msg->GetTo(), msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    else if (t == MSG_PRIVATE_RECEIVED)
    {
      _AppendSB(CLR_MAGENTA_BG, 5);
      _AppendSB(CLR_BLACK_FG, 5);
      buffer_length = snprintf(buf, sizeof(buf), "<- %s: %s", msg->GetFrom(),
                               msg->GetBody());

      if (buffer_length >= sizeof(buf))
      {
        buffer_length = sizeof(buf) - 1;
      }
    }

    num_of_lines += _PrAppendSB(buf, buffer_length);
    _AppendSB(CLR_DEFAULT, 4);

    if (num_of_lines > term.screen_rows - 3)
    {
      break;
    }
  }

  for (unsigned int i = num_of_lines; i < term.screen_rows - 1; i++)
  {
    _PrAppendSB(" ", 1);
  }
}

void NcTTyUi::_DisplayConnecting()
{
  unsigned int num_of_lines = 0;

  _AppendSB(" Connecting user '", 18);
  _AppendSB(username, strlen(username));
  _AppendSB("' with password '", 17);
  _AppendSB(password, strlen(password));
  _AppendSB("'...", 4);

  for (unsigned int i = num_of_lines; i < term.screen_rows - 1; i++)
  {
    _PrAppendSB(" ", 1);
  }

  NetworkStatus connect_status = net->Connect();

  if (connect_status == NO_NET_ERROR)
  {
    connected = true;

    char connect_string[MAX_PROMPT + 1];

    if (strlen(password) > 0)
    {
      sprintf(connect_string, ".n %s=%s", username, password);
    }

    else
    {
      sprintf(connect_string, ".n %s", username);
    }

    const char *connect_message = "\% Connected using nctty...";

    SSL_write(net->GetSSL(), connect_string, strlen(connect_string) + 1);
    SSL_write(net->GetSSL(), connect_message, strlen(connect_message) + 1);

    std::thread receive_thread([&]() { net->ReceiveData(this); });
    receive_thread.detach();
  }

  else
  {
    _Fatal(NetStatusToText(connect_status));
  }
}

void NcTTyUi::_DisplayDisconnecting()
{
  unsigned int num_of_lines = 0;

  num_of_lines += _PrAppendSB("    Disconnecting.. Please wait!", 32);

  for (unsigned int i = num_of_lines; i < term.screen_rows - 1; i++)
  {
    _PrAppendSB(" ", 1);
  }
}

void NcTTyUi::_PrintPrompt()
{
  in_insert_mode ? _AppendSB("I", 1) : _AppendSB("C", 1);
  _AppendSB(">> ", 3);
  _PromptToSB();
  _AppendSB(CLEAR_TO_END, 3);
}

void NcTTyUi::_RefreshScreen()
{
  term.screen_buffer[0] = '\0';
  term.buffer_length = 0;
  _AppendSB(HIDE_CURSOR, 6);
  _AppendSB(HOME_POSITION, 3);

  if (term.screen_rows < MIN_TERM_HEIGHT || term.screen_cols < MIN_TERM_WIDTH)
  {
    _DisplayTermWidthWarning();
  }

  else if (get_username || get_password)
  {
    _DisplayWelcome();
  }

  else if (connected && !disconnecting)
  {
    accept_input = true;
    _DisplayMessages();
  }

  else if (disconnecting)
  {
    accept_input = false;
    _DisplayDisconnecting();
  }

  else
  {
    accept_input = false;
    _DisplayConnecting();
  }

  _PrintPrompt();

  // reposition the cursor
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", term.cur_y + 1, term.cur_x + 1);
  _AppendSB(buf, strlen(buf));

  _AppendSB(SHOW_CURSOR, 6);

  in_insert_mode ? _AppendSB(CURSOR_INSERT, 5) : _AppendSB(CURSOR_MOVE, 5);
  write(STDOUT_FILENO, term.screen_buffer, term.buffer_length);
}

void NcTTyUi::_AddMessageToMessages(NcMessage *msg)
{
  if (number_of_messages < MAX_MESSAGES)
  {
    all_messages[number_of_messages++].SetMessage(msg);
  }

  else
  {
    for (size_t i = 1; i < number_of_messages; i++)
    {
      all_messages[i - 1].SetMessage(&all_messages[i]);
    }

    all_messages[number_of_messages - 1].SetMessage(msg);
  }

  _RefreshScreen();
}

size_t NcTTyUi::_Trim(char *str)
{
  size_t num_of_chars = 0;
  char *start = str;

  while (isspace((unsigned char)*start))
  {
    ++start;
    num_of_chars++;
  }

  char *end = start + strlen(start) - 1;

  while (end > start && isspace((unsigned char)*end))
  {
    --end;
    num_of_chars++;
  }

  *(end + 1) = '\0';

  if (start != str)
  {
    memmove(str, start, (end - start) + 2);
  }

  return num_of_chars;
}

void NcTTyUi::_Fatal(const char *s)
{
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[3J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  app_has_error = true;
  app_is_running = false;
}

void NcTTyUi::_CleanUp()
{
  if (net != NULL)
  {
    delete net;
  }

  if (term.screen_buffer != NULL)
  {
    free(term.screen_buffer);
  }

  if (prompt != NULL)
  {
    free(prompt);
  }
}
