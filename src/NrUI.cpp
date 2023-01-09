/*
 * File:   NrUI.cpp
 * Author: Michel Alwan
 *
 * Created on 2022-12-31
 */

#include "NrUI.h"
#include "NrNet.h"
#include "NrIO.h"

NrUI *obj;

/*
 * Public Functions
 */

NrUI::NrUI(NrString username, NrString password) :
  io                 {new NrIO(this, 0, 0)},
  username           {username},
  password           {password},
  term_resized       {false},
  term_resized_input {false},
  exit_app           {false},
  connected          {false},
  messages_count     {0},
  display_page       {0}
{
  all_messages = new NrString[MAX_MESSAGES];
  net          = new NrNet();

  NetworkStatus connect_status = net->Connect();

  switch(connect_status)
  {
    case NO_NET_ERROR:
    {
      connected = true;

    } break;

    default:
    {
      exit_message = NetStatusToText(connect_status);
      exit_app = true;

    } break;
  }
}

NrUI::~NrUI()
{
  delete io;
  delete net;
  delete[] all_messages;
}

void NrUI::sigwinch_handler(int signum)
{
  term_resized       = true;
  term_resized_input = true;
}

void sigwinch_wrapper(int signum)
{
  obj->sigwinch_handler(signum);
}

void NrUI::Run()
{
  obj = this;

  std::thread receive_thread;
  std::thread update_thread;

  signal(SIGWINCH, sigwinch_wrapper);

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  max_rows = w.ws_row;
  max_cols = w.ws_col;

  bool run_once = true;
  bool ui_update_once = true;

  while (true)
  {
    if (ui_update_once)
    {
      ui_update_once = false;
      
      update_thread = std::thread([&]() { UpdateTerminal(); });
      update_thread.detach();
    }

    if (!connected)
    {
      exit_message = "Disconnected from Server.. :(";
      exit_app = true;
    }

    if (exit_app)
    {
      system("clear");
      MoveTo(1, 1);
      
      if (connected)
      {
        printf("Closing Connection!\n");
        net->StopReceiving();
        while (!net->FinishedReceiving()) { sleep(1); }
      }
      
      sleep(1);
      MoveTo(3, 3);
      exit_message.Echo();
      printf("\n");
      break;
    }

    if (run_once)
    {
      NrString connect_string = NrString(".n ") + username;
      if (password.GetLength() > 0) { connect_string = connect_string + '=' + password; }
      SSL_write(net->GetSSL(), connect_string.ToString(), connect_string.GetLength() + 1);

      run_once = false;
      std::thread receive_thread([&]() { net->ReceiveData(this); });
      receive_thread.detach();
    }

    DrawUI();
  }
}

void NrUI::AddBufferToMessages(const char* buffer, size_t bufsize)
{
  NrString nrs_buffer;
  NrString msg;
  
  for (size_t i = 0; i < bufsize; i++)
  {
    nrs_buffer.InsertCharAt(buffer[i], nrs_buffer.GetLength());
  }
  
  while (true)
  {
    int newline_pos = nrs_buffer.HaveChar('\n');
    if (newline_pos == -1) { break; }
    msg = nrs_buffer.SubString(0, newline_pos - 1);
    nrs_buffer = nrs_buffer.SubString(newline_pos + 1, nrs_buffer.GetLength()-1);
    if (msg.GetLength() > 0)
    {
      AddMessageToMessages(msg);
    }
  }

  if (nrs_buffer.GetLength() > 0) { AddMessageToMessages(nrs_buffer); }
}

void NrUI::SetFlagPageUp()
{
  if (display_page < messages_count - 1)
  {
    display_page++;
    UpdateDisplay();
  }
}

void NrUI::SetFlagPageDn()
{
  if (display_page > 0)
  {
    display_page--;
    UpdateDisplay();
  }
}

/*
 * Private Functions
 */

void NrUI::UpdateDisplay()
{
  long grid = max_cols * max_rows;
  int row = prompt_height + 5;
  char c;
  int str_index;
  int written_letters = 0;
  bool running = true;
  int start_message_at;

  for (int j = prompt_height + 6;
           j < prompt_height + display_height;
           j++)
  {
    MoveTo(j, 1);
    for (int k = 0; k < max_cols; k++) { putchar(' '); }
  }

  StartDebug(max_cols);
  SetColor(CLR_RED_FG);
  
  if (display_page != 0)
  {
    for (int i = 0; i < max_cols - 32; i++) { putchar(' '); }
    printf("...Scroll Down for Most Recent!");
  }
  
  else
  {
    for (int i = 0; i < max_cols; i++) { putchar(' '); }
  }

  MoveDown(1);
  printf("\r");
  for (int i = 0; i < max_cols; i++) { printf("─"); }
  
  SetColor(CLR_DEFAULT);
  RestorePosition();
  
  for (size_t i = display_page; i < messages_count; i++)
  {
    if (!running) { break; }
    NrString message = all_messages[i];
    start_message_at = 0;

    if (message.GetCharAt(0) == '>' && message.GetCharAt(1) == '>')
    {
      SetColor(CLR_RED_FG);

      if
      (    message.GetCharAt(3) == 'M'
        && message.GetCharAt(4) == 'e'
        && message.GetCharAt(5) == 's'
        && message.GetCharAt(6) == 's'
        && message.GetCharAt(7) == 'a'
        && message.GetCharAt(8) == 'g'
        && message.GetCharAt(9) == 'e')

      {
        start_message_at = message.HaveChar('[');
        if (start_message_at == -1) { start_message_at = 0; }
        SetColor(CLR_MAGENTA_FG);
      }
    }

    if (message.GetCharAt(0) == '<')
    {
      SetColor(CLR_MAGENTA_BG);
      SetColor(CLR_BLACK_FG);
    }

    if (message.GetCharAt(0) == '(')
    {
      SetColor(CLR_GREEN_BG);
      SetColor(CLR_BLACK_FG);
    }

    if (message.GetCharAt(0) == '[')
    {
      size_t i = 0;

      while (true)
      {
        if (i == message.GetLength())    { break; }
        if (message.GetCharAt(i) != ']') { i++;   }
        else                             { break; }
      }

      i++;
      size_t name_index = i;
      bool colorise = true;

      while (true)
      {
        if (username.GetCharAt(i - name_index) == '\0') { break; }
        
        if (message.GetCharAt(i) != username.GetCharAt(i - name_index))
        {
          colorise = false; 
          break;
        }
        
        i++;
      }

      if (message.GetCharAt(i) != ':') { colorise = false;      }
      if (colorise)                    { SetColor(CLR_BLUE_FG); }
    }

    row++;
    written_letters = 0;
    MoveTo(row, 1);
    str_index = start_message_at;

    while (true)
    {
      c = message.GetCharAt(str_index);

      if (grid - row * max_cols - written_letters <= 0)
      {
        running = false;
        break;
      }

      if (c == '\0') { break; }

      if (written_letters == max_cols)
      {
        written_letters = 0;
        row++;
        MoveTo(row, 1);
      }

      putchar(c);
      str_index++;
      written_letters++;
    }

    SetColor(CLR_DEFAULT);
  }
}

void NrUI::AddMessageToMessages(NrString msg)
{
  size_t start = messages_count;
  if (messages_count == MAX_MESSAGES) { start--; }
  else { messages_count++; }
  
  for (size_t i = start; i > 0; i--)
  {
    all_messages[i] = all_messages[i - 1];
  }

  all_messages[0] = msg;

  UpdateDisplay();
  MoveTo(7, 2);
  printf("Received: %6ld Bytes.", msg.GetLength());
  MoveUp(1);
  printf("\n");
}

void NrUI::DrawUI()
{
  ResetBoundaries();

  io->SetMaxLength(io_limits);
  io->SetPromptHeight(prompt_height);
  
  ClrScr();
  DrawPromptBox();
  UpdateDisplay();
  ResetCursorToPrompt();
  io->ReadString();

  NrString io_line = io->GetIoLine();
  io_line.Clean();
  
  if (io_line == ":q" || io_line == "/quit" || io_line == ".q")
  {
    exit_app = true;
  }

  else if
  ((
        io_line.GetCharAt(0)    == '.'
        && io_line.GetCharAt(1) == 'n'
        && io_line.GetCharAt(2) == ' '
   ) || (
        io_line.GetCharAt(0)    == '.'
        && io_line.GetCharAt(1) == 'p'
        && io_line.GetCharAt(2) == ' '
   ) || (
        io_line == ".w" || io_line == ".e" || io_line == ".d"
   ) || (
        io_line.GetCharAt(0)    != '/'
        && io_line.GetCharAt(0) != '.'
  ))
  {
    if (connected)
    {
      SSL_write
      (
        net->GetSSL(),
        io_line.ToString(),
        io_line.GetLength() + 1
      );
    }

    else
    {
      exit_message = "Disconnected from Server.. :(";
      exit_app = true;
    }
  }
}

void NrUI::ResetBoundaries()
{
  width = max_cols - 2;
  prompt_height= 4;
  display_height = max_rows - prompt_height - 1;
  io_limits = max_cols - 4;
}

void NrUI::DrawPromptBox()
{
  SetColor(CLR_RED_FG);
  MoveTo(1, 1);
  SavePosition();

  printf("╭");
  for (int i = 0; i < width; i++) { printf("─"); }
  printf("╮");

  RestorePosition();
  MoveDown(1);
  
  for (int i = 0; i < prompt_height; i++)
  {
    printf("│");
    MoveRight(width);
    printf("│");
    MoveLeft(width + 2);
    MoveDown(1);
  }

  printf("╰");
  for (int i = 0; i < width; i++) { printf("─"); }
  printf("╯");
  SetColor(CLR_DEFAULT);
}

void NrUI::UpdateTerminal()
{
  while (!exit_app)
  {
    if (term_resized)
    {
      term_resized = false;

      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
      max_rows = w.ws_row;
      max_cols = w.ws_col;
      fflush(stdout);
      DrawUI();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
