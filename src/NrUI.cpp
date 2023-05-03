/*
 * File:   NrUI.cpp
 * Author: Michel Alwan
 *
 * Created on 2023-01-30
 */

#include "NrUI.h"
#include "NrNet.h"
#include "NrIO.h"

std::string wrap_string(const std::string &str, uint16_t width)
{
  std::string wrapped_string;
  uint16_t line_start = 0;
  uint16_t line_end = width;
    
  while (line_end < str.length())
  {
    while (str[line_end] != ' ' && line_end > line_start) { line_end--; }

    wrapped_string += str.substr(line_start, line_end - line_start) + "\n";
    line_start = line_end + 1;
    line_end = line_start + width;
  }

  wrapped_string += str.substr(line_start);

  return wrapped_string;
}

NrUI *obj;

NrUI::NrUI(std::string username, std::string password) :
  username     {username},
  password     {password},
  term_resized {false},
  exit_app     {false},
  connected    {false}
{
  exit_message = "Application ended in peace!";

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  rows = w.ws_row;
  cols = w.ws_col;
  display_height = rows - PROMPT_HEIGHT - 1;
  scr.ClrScr();

  io = new NrIO(cols);
  net = new NrNet();

  NetworkStatus connect_status = net->Connect();

  switch(connect_status)
  {
    case NO_NET_ERROR:
    {
      connected = true;
      message_buffer = "";

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
}

void NrUI::sigwinch_handler(int signum)
{
  term_resized = true;
}

void sigwinch_wrapper(int signum)
{
  obj->sigwinch_handler(signum);
}

void NrUI::Run()
{
  obj = this;
  std::thread update_terminal_thread;
  signal(SIGWINCH, sigwinch_wrapper);
 
  update_terminal_thread = std::thread([&]() { UpdateTerminal(); });
  update_terminal_thread.detach();
  
  std::string connect_string;

  if (password.length() > 0)
  {
    connect_string = ".n " + username + "=" + password;
  }

  else
  {
    connect_string = ".n " + username;
  }

  std::string connect_message = "\% Connected using ncrev...";

  bool run_once = true;
  
  while (true)
  {
    if (!connected)
    {
      exit_message = "Disconnected from Server.. :(";
      exit_app = true;
    }

    if (exit_app)
    {
      scr.ClrScr();
      scr.MoveTo(1, 1);
      scr.ShowCursor();

      if (connected)
      {
        printf("Closing Connection!\n");
        net->StopReceiving();
        while (!net->FinishedReceiving()) { sleep(1); }
      }
      
      sleep(1);
      scr.MoveTo(3, 3);
      printf("%s\n", exit_message.c_str());

      break;
    }

    if (run_once)
    {
      run_once = false;
      SSL_write(net->GetSSL(), connect_string.c_str(), connect_string.length() + 1);
      SSL_write(net->GetSSL(), connect_message.c_str(), connect_message.length() + 1);
      std::thread receive_thread([&]() { net->ReceiveData(this); });
      receive_thread.detach();
    }

    DrawUI();
  }
}

void NrUI::AddBufferToMessages(const char* buffer, int bytes_received)
{
  std::string message_string = "";

  message_buffer.append(buffer, bytes_received);

  for (int i = 0; i < bytes_received; i++)
  {
    if (buffer[i] == '\0' || buffer[i] == '\r') { continue; }

    if (buffer[i] == '\n')
    {
      message_string += '\0';
      AddMessageToMessages(StringToMessage(message_string));
      message_buffer.erase(0, i + 1);
      message_string = "";
    }
    
    else { message_string += buffer[i]; }
  }
}

void NrUI::SetFlagPageUp()
{

}

void NrUI::SetFlagPageDn()
{

}

/*****************************************************************************/

void NrUI::UpdateDisplay()
{
  scr.ClrScr();
  scr.MoveTo(1, 1);

  for (uint16_t i = 0; i < all_messages.size(); i++)
  {
    PrintMessage(all_messages[i]);
  }
      if (io->GetIoLine().length() > 0)
      {
        io->Echo();
        scr.MoveTo(3, 1);
        printf("\n");
      }

}

void NrUI::UpdateTerminal()
{
  while (!exit_app)
  {
    if (term_resized)
    {
      term_resized = false;
      scr.ClrScr();

      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
      rows = w.ws_row;
      cols = w.ws_col;
      display_height = rows - PROMPT_HEIGHT - 1;
      io->SetWidth(cols);
      
      fflush(stdout);
      io->ClearPrompt();
      UpdateDisplay();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void NrUI::DrawUI()
{
  io->SetWidth(cols);
  scr.ClrScr();
  UpdateDisplay();
  io->ReadInput();

  std::string io_line = io->GetIoLine();
  io_line.erase(0, io_line.find_first_not_of(" \t\n\r"));
  io_line.erase(io_line.find_last_not_of(" \t\n\r") + 1);

  if (io_line == ":q" || io_line == ".q" || io_line == "/quit")
  {
    exit_app = true;
  }

  else if
  ((
        io_line[0]    == '.'
        && io_line[1] == 'n'
        && io_line[2] == ' '
   ) || (
        io_line[0]    == '.'
        && io_line[1] == 'p'
        && io_line[2] == ' '
   ) || (
        io_line == ".w" || io_line == ".e" || io_line == ".d"
   ) || (
        io_line[0]    != '/'
        && io_line[0] != '.'
  ))
  {
    if (connected)
    {
      SSL_write
      (
        net->GetSSL(),
        io_line.c_str(),
        io_line.length() + 1
      );
    }

    else
    {
      exit_message = "Disconnected from Server.. :(";
      exit_app = true;
    }
  }
}

void NrUI::AddMessageToMessages(NrMessage msg)
{
  if (all_messages.size() == BUFSIZE)
  {
      all_messages.erase(all_messages.begin());
  }
  
  all_messages.push_back(msg);
  UpdateDisplay();

}

NrMessage NrUI::StringToMessage(std::string message_string)
{
  size_t start_uname, end_uname;
  size_t start_from, end_from, start_to, end_to, start_body;
  NrMessage msg;

  if (message_string[0] == '<')
  {
    msg.type = MSG_PRIVATE_RECEIVED;

    start_from = message_string.find("<");
    end_from   = message_string.find(" (private):");
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
    end_uname   = message_string.find(":");
    std::string uname =
      message_string.substr(start_uname, end_uname - start_uname);

    if (uname == username) { msg.type = MSG_SENT;     }
    else                   { msg.type = MSG_RECEIVED; }

    start_from = message_string.find("[");
    end_from   = message_string.find(":");
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

void NrUI::PrintMessage(NrMessage msg)
{
  std::string formatted;

  scr.SetColor(CLR_DEFAULT);

  switch (msg.type)
  {
    case MSG_NONE:
    {
      formatted = msg.body;
      scr.SetColor(CLR_DEFAULT);

    } break;

    case MSG_SENT:
    {
      formatted = msg.from + ": " + msg.body;
      scr.SetColor(CLR_BLUE_FG);

    } break;

    case MSG_RECEIVED:
    {
      formatted = msg.from + ": " + msg.body;
      scr.SetColor(CLR_DEFAULT);

    } break;

    case MSG_PRIVATE_SENT:
    {
      formatted = "-> " + msg.to + ": " + msg.body;
      scr.SetColor(CLR_MAGENTA_FG);

    } break;

    case MSG_PRIVATE_RECEIVED:
    {
      formatted = "<- " + msg.from + ": " + msg.body;
      scr.SetColor(CLR_MAGENTA_BG);
      scr.SetColor(CLR_BLACK_FG);

    } break;

    case MSG_EMOTE:
    {
      formatted = msg.body;
      scr.SetColor(CLR_GREEN_BG);
      scr.SetColor(CLR_BLACK_FG);

    } break;

    case MSG_SYSTEM:
    {
      formatted = msg.body;
      scr.SetColor(CLR_RED_FG);

    } break;
  }

  printf("%s", wrap_string(formatted, cols).c_str());
  
  scr.SetColor(CLR_DEFAULT);
  printf("\n");

}
