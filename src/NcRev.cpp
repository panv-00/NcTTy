/*
 * File:   NcRev.cpp
 * Author: Michel Alwan
 *
 * Created on 2022-12-31
 */

#include "NrUI.h"
#include "NrString.h"

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printf("NcRev v1.0.0\n");
    printf("Usage:\n\n   $ ./ncrev [username]\nor $ ./ncrev [username=password]\n");

    return 1;
  }

  NrString prog_arg, username, password;
  
  prog_arg = argv[1];
  int pos = prog_arg.HaveChar('=');
  
  if (pos != -1)
  {
    username = prog_arg.SubString(0, pos - 1);
    password = prog_arg.SubString(pos + 1, prog_arg.GetLength() - 1);
  }

  else
  {
    username = prog_arg;
    password = "";
  }
  
  NrUI ui(username, password);
  ui.Run();

  return 0;
}
