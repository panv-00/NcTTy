/*
 * File:   NrIO.cpp
 * Author: Michel Alwan
 *
 * Created on 2023-01-31
 */

#include "NrIO.h"

NrIO::NrIO(uint16_t width) :
  width {width},
  cursor_row {0},
  cursor_col {0},
  io_cursor {0}
{
  ClearPrompt();
}

NrIO::~NrIO()
{

}

void NrIO::ReadInput()
{
  int a = 0;
  bool do_insert;

  io_line = "";
  io_cursor = 0;

  ClearPrompt();

  while((a = Getch()) && IsAllowed(a))
  {
    do_insert = true;
    if (io_line.length() >= BUFSIZE) { do_insert = false; }

    if (a == '\n') { return; }

    if (a == BACKSPACE)
    {
      do_insert = false;
      
      if (io_cursor > 0)
      {
        io_cursor--;
        io_line.erase(io_cursor, 1);
      }
    }

    if (a == ESCAPE_CHAR)
    {
      a = Getch();

      if (a == ARROW_CHAR)
      {
        do_insert = false;
        a = Getch();

        if (a == ARROW_UP) {}
        if (a == ARROW_DN) {}
        if (a == ARROW_RT) { if (io_cursor < io_line.length()) { io_cursor++; } }
        if (a == ARROW_LT) { if (io_cursor > 0) { io_cursor--; } }

        if (a == PAGE_UP)
        {
          if (Getch() == PAGE_UP_DN)
          {
            //scr.ScrollUp();
          }
        }

        if (a == PAGE_DN)
        {
          if (Getch() == PAGE_UP_DN)
          {
            //scr.ScrollDn();
          }
        }
      }
    }
    
    if (do_insert)
    {
      io_line.insert(io_cursor, 1, a);
      io_cursor++;
    }

    Echo();
    a = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////

void NrIO::ClearPrompt()
{
  scr.SetColor(CLR_GREEN_FG);
  scr.MoveTo(1, 1);

  for (uint8_t i = 0; i < PROMPT_HEIGHT + 2; i++)
  {
    for (uint16_t j = 0; j < width; j++) { putchar(' '); }
    printf("\n");
  }
  
  scr.MoveTo(PROMPT_HEIGHT + 1, 1);
  
  for (int i = 0; i < width; i++) putchar('_');

  scr.MoveTo(1, 2);
  scr.SetColor(CLR_DEFAULT);
}

void NrIO::ParseString()
{
  std::vector<std::string> body;
  char delimiter = ' ';

  std::stringstream ss(io_line);
  std::string word;

  body.clear();

  while (std::getline(ss, word, delimiter))
  {
    body.push_back(word);
  }

  cursor_col = 0;

  for (const auto &word : body)
  {
    if (lines.empty() || (cursor_col + word.length() + 2 > width))
    {
      lines.push_back("");
      cursor_col = 0;
    }
    
    lines.back() += word + " ";
    cursor_col += word.length() + 1;
  }
}

void NrIO::Echo()
{
  ClearPrompt();
  scr.ShowCursor();
  lines.clear();
  ParseString();

  cursor_col = 0;
  cursor_row = 0;

  uint8_t start = 0;
  if (lines.size() > PROMPT_HEIGHT) { start = lines.size() - PROMPT_HEIGHT; }
 
  size_t counter = io_cursor;
  while (counter > lines[cursor_row].length())
  {
    counter -= lines[cursor_row].length();
    cursor_row++;
  }

  if (cursor_row < PROMPT_HEIGHT)
  {
    cursor_row++;
  }

  if (cursor_row > PROMPT_HEIGHT) { cursor_row = PROMPT_HEIGHT; }
  
  cursor_col = counter + 2;

  if (cursor_col > width)
  {
    if (cursor_row < PROMPT_HEIGHT)
    {
      cursor_row++;
    }
    
    else
    {
      start++;
    }
    
    cursor_col = 2;
  }

  scr.SetColor(CLR_GREEN_FG);

  for (uint8_t i = start; i < lines.size(); i++)
  {
    scr.MoveTo(i + 1 - start, 2);
    printf("%s", lines[i].c_str());
  }

  scr.MoveTo(cursor_row, cursor_col);
  scr.SetColor(CLR_DEFAULT);
}

int NrIO::Getch()
{ 
  struct termios oldt, newt;
  int ch;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return ch;
}

bool NrIO::IsAllowed(int c)
{
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return true;

  if (c >= '0' && c <= '9') return true;

  switch (c)
  {
    case '`':
    case '~':
    case '!':
    case '@':
    case '#':
    case '$':
    case '%':
    case '^':
    case '&':
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case '.':
    case ',':
    case '<':
    case '>':
    case ';':
    case '?':
    case ':':
    case '_':
    case '|':
    case '+':
    case '-':
    case '*':
    case '/':
    case '=':
    case '"':
    case '\\':
    case '\'':
    case ' ':
    case '\n':
      return true;
  }
  
  switch (c)
  {
    case PAGE_UP:
    case PAGE_DN:
    case PAGE_UP_DN:
    case BACKSPACE:
    case ESCAPE_CHAR:
    case ARROW_CHAR:
    case ARROW_RT:
    case ARROW_LT:
    case ARROW_UP:
    case ARROW_DN:
      return true;
  }
  
  return false;
}

