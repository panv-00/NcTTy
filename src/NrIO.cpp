/*
 * File:   NrIO.cpp
 * Author: Michel Alwan
 *
 * Created on 2022-12-31
 */

#include "NrIO.h"

/*
 * Non-Class Functions
 */

void ClrScr         (            ) { system("clear"            ); };
void SavePosition   (            ) { printf("\x1b%d",         7); };
void RestorePosition(            ) { printf("\x1b%d",         8); };
void SetColor       (ColorCode cc) { printf("\x1b[%dm",      cc); };
void MoveUp         (int count   ) { printf("\x1b[%dA",   count); };
void MoveDown       (int count   ) { printf("\x1b[%dB",   count); };
void MoveRight      (int count   ) { printf("\x1b[%dC",   count); };
void MoveLeft       (int count   ) { printf("\x1b[%dD",   count); };
void MoveTo         (int r, int c) { printf("\x1b[%d;%df", r, c); };

void ResetCursorToPrompt()
{
  MoveTo (2, 1);
  printf(START_INPUT);
}

// Call RestorePosition once debug is over.
void StartDebug(int max_cols)
{
  SavePosition();
  MoveTo(7, 1);
  
  for (int i = 0; i < max_cols; i++) { putchar (' '); }
  
  MoveTo(7, 1);
}

/*
 * Public Functions
 */

NrIO::NrIO(NrUI *nrui, int max_length, int prompt_height) :
  nrui            {nrui},
  io_line         {""},
  max_length      {max_length},
  prompt_height   {prompt_height},
  cursor_position {0}
{

}

NrIO::~NrIO()
{

}

void NrIO::ReadString()
{
  int  a = 0;
  bool do_insert;
  
  io_line = "";
  cursor_position = 0;
  size_t line_length = 0;
  
  while ((a = Getch()) && (IsAlpha(a) || IsNum(a) || IsPunct(a) || IsCtrl(a)))
  {

    do_insert = true;

    line_length = io_line.GetLength();

    if (a == '\n') { return; }

    if (a == BACKSPACE)
    {
      do_insert = false;

      if (cursor_position > 0)
      {
        cursor_position--;
        io_line.DeleteCharAt(cursor_position);
      }
    }
      
    if (a == ESCAPE_CHAR)
    {
      a = Getch();
     
      if (a == ARROW_CHAR)
      {
        do_insert = false;
        a = Getch();
        
        if (a == ARROW_UP)
        {
          if (cursor_position >= (size_t) max_length)
          {
            cursor_position -= max_length;
          }
        }
        
        if (a == ARROW_DN)
        {
          if (io_line.GetLength() >= (size_t) max_length)
          {
            if (cursor_position <= (size_t) (io_line.GetLength() - max_length))
            {
              cursor_position += max_length;
            }
          }
        }
        
        if (a == ARROW_RT)
        {
          if (cursor_position < io_line.GetLength())
          {
            cursor_position++;
          }
        }
        
        if (a == ARROW_LT)
        {
          if (cursor_position > 0) { cursor_position--; }
        }
          
        if (a == PAGE_UP)
        {
          if (Getch() == PAGE_UP_DN) { nrui->SetFlagPageUp(); }
        }
         
        if (a == PAGE_DN)
        {
          if (Getch() == PAGE_UP_DN) { nrui->SetFlagPageDn(); }
        }
      }
    }

    if (do_insert)
    {
      io_line.InsertCharAt(a, cursor_position);
      cursor_position++;
      if (cursor_position > io_line.GetLength())
      {
        cursor_position = io_line.GetLength();
      }
    }

    Echo(line_length);
    a = 0;

    if (nrui->GetTermResizedInput())
    {
      nrui->ResetTermResizedInput();
      io_line = "";
      line_length = 0;
      return;
    }

  }
}
  

/*
 * Private Functions
 */

void NrIO::Echo(size_t line_length)
{
  int io_length = io_line.GetLength();
  int number_of_lines = io_length / max_length;
  int start = number_of_lines - prompt_height;
 
  if ((int) (cursor_position / max_length) > start)
  {
    start = cursor_position / max_length - prompt_height + 1;
  }

  if (start < 0) { start = 0; }

  for (int i = start; i < start + prompt_height; i++)
  {
    ResetCursorToPrompt();

    if (i - start > 0) { MoveDown(i - start); }

    printf(START_INPUT);
    for (int i = 0; i < max_length; i++) { putchar (' '); }
    printf(START_INPUT);

    for(int j = 0; j < max_length; j++)
    {
      putchar(io_line.GetCharAt(i * max_length + j));
    }
  }
  
  ResetCursorToPrompt();
  
  int move_cursor_down = 0;
  
  if (cursor_position / max_length - start > 0)
  {
    move_cursor_down = cursor_position / max_length - start;
  }

  if (move_cursor_down > prompt_height - 1)
  {
    move_cursor_down = prompt_height - 1;
  }

  if (move_cursor_down > 0)
  {
    MoveDown(move_cursor_down);
  }

  if (cursor_position % max_length > 0)
  {
    MoveRight(cursor_position % max_length);
  }
}

int NrIO::Getch(void)
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

bool NrIO::IsAlpha(int c)
{
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return true;
  return false;
}

bool NrIO::IsNum(int c)
{
  if (c >= '0' && c <= '9') return true;
  return false;
}

bool NrIO::IsPunct(int c)
{
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
      return true;

    default:
      return false;
  }
}

bool NrIO::IsCtrl(int c)
{
  switch (c)
  {
    case ' ':
    case '\n':
    case BACKSPACE:
    case ESCAPE_CHAR:
    case ARROW_CHAR:
    case ARROW_RT:
    case ARROW_LT:
    case ARROW_UP:
    case ARROW_DN:
      return true;

    default:
      return false;
  }
}
