/*
 * File:   NcRev.cpp
 * Author: Michel Alwan
 *
 * Created on 2023-01-30
 */

#include "NrUI.h"

int main(int argc, char** argv)
{
  NrScreen scr;
  scr.ClrScr();

  printf("NcRev v%s\n", VERSION);
  
  if (argc != 2)
  {
    printf("Usage:\n\n");
    printf("    $ ./ncrev [username]\n");
    printf("or  $ ./ncrev [username]=[password]\n");

    return 1;
  }

  printf("...Connecting, please wait!\n");
  sleep(1);
  
  std::string input = argv[1];
  std::string delimiter = "=";
  std::string username, password;

  size_t pos = input.find(delimiter);

  if (pos == 0)
  {
    fprintf(stderr, "Halt. Username cannot be empty!\n");
    return 1;
  }

  else if (pos == std::string::npos)
  {
    username = input;
    password = "";
  }

  else
  {
    username = input.substr(0, pos);
    password = input.substr(pos + delimiter.length());
  }

  NrUI *ui = new NrUI(username, password);
  ui->Run();

  delete ui;

  return 0;
}
