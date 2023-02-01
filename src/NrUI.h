/*
 * File:   NrUI.h
 * Author: Michel Alwan
 *
 * Created on 2023-01-30
 */

#ifndef NRUI_H
#define NRUI_H

#include "NrInclude.h"
#include "NrScreen.h"

class NrNet;
class NrIO;

typedef struct
{
  MessageType type;
  std::string from;
  std::string to;
  std::string body;

} NrMessage;

class NrUI
{
public:
  NrUI(std::string username, std::string password);
  ~NrUI();

  void sigwinch_handler(int signum);

  void Run();
  void AddBufferToMessages(const char* buffer, int bytes_received);

  void SetFlagPageUp();
  void SetFlagPageDn();

  void EndConnection() { connected = false; };

protected:

private:
  friend void sigwinch_wrapper(int signum);

  void UpdateDisplay();
  void UpdateTerminal();
  void DrawUI();
  void AddMessageToMessages(NrMessage msg);
  NrMessage StringToMessage(std::string message_string);
  void PrintMessage(NrMessage msg);

  std::string username, password;

  bool term_resized;

  struct winsize w;
  int rows;
  int cols;
  int display_height;

  bool exit_app;
  bool connected;
  
  std::string exit_message;

  NrIO *io;
  NrNet *net;
  NrScreen scr;

  std::vector<NrMessage> all_messages;
  std::string message_buffer;

};

#endif
