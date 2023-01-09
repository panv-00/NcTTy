/*
 * File:   NrUI.h
 * Author: Michel Alwan
 *
 * Created on 2022-12-31
 */

#ifndef NRUI_H
#define NRUI_H

#include "NrInclude.h"
#include "NrString.h"

class NrNet;
class NrIO;

class NrUI
{
public:
  NrUI(NrString username, NrString password);
  ~NrUI();

  void sigwinch_handler(int signum);
  void Run();

  void AddBufferToMessages(const char* buffer, size_t bufsize);
  void UpdateDisplay();
  
  int    GetMaxCols()              { return max_cols;           };
  size_t GetMessagesCount()        { return messages_count;     };
  bool   GetTermResized()          { return term_resized;       };
  bool   GetTermResizedInput()     { return term_resized_input; };

  void ResetTermResizedInput() 
  {
    term_resized_reset_count--;
    
    if (term_resized_reset_count == 0)
    {
      term_resized_reset_count = 2;
      term_resized_input = false;
    }
  };

  void SetFlagPageUp();
  void SetFlagPageDn();

  void EndConnection() { connected = false; };

protected:

private:
  friend void sigwinch_wrapper(int signum);

  void AddMessageToMessages(NrString msg);
  
  void DrawUI();
  void ResetBoundaries();
  void DrawPromptBox();
  void UpdateTerminal();

  NrIO *io;

  NrString username;
  NrString password;

  NrString *all_messages;

  NrString exit_message = "Application Ended in Peace!";
  
  bool   term_resized;
  bool   term_resized_input;
  int    term_resized_reset_count = 2;
  bool   exit_app;
  bool   connected;
  size_t messages_count;

  struct winsize w;
  int max_rows;
  int max_cols;
  int width;
  int display_height;
  int prompt_height;
  int io_limits;

  bool   page_up_pressed;
  bool   page_dn_pressed;
  size_t display_page;

  NrNet *net;
};

#endif
