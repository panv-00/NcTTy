/*
 * File:   NrIO.h
 * Author: Michel Alwan
 *
 * Created on 2023-01-31
 */

#ifndef NRIO_H
#define NRIO_H

#include "NrInclude.h"
#include "NrScreen.h"

class NrIO
{
public:
  NrIO(uint16_t width);
  ~NrIO();

  void ReadInput();
  void SetWidth(uint16_t w) { width = w; };

  std::string GetIoLine() { return io_line; };

protected:

private:
  void ClearPrompt();
  void ParseString();
  void Echo();
  int Getch();
  bool IsAllowed(int c);

  std::string io_line;
  std::vector<std::string> lines;

  uint16_t width;
  uint16_t cursor_row;
  uint16_t cursor_col;
  size_t io_cursor;

  NrScreen scr;
};

#endif
