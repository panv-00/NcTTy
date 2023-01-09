/*
 * File:   NrIO.h
 * Author: Michel Alwan
 *
 * Created on 2022-12-31
 */

#ifndef NRIO_H
#define NRIO_H

#include "NrInclude.h"
#include "NrString.h"
#include "NrUI.h"

void ClrScr         (            );
void SavePosition   (            );
void RestorePosition(            );
void SetColor       (ColorCode cc);
void MoveUp         (int count   );
void MoveDown       (int count   );
void MoveRight      (int count   );
void MoveLeft       (int count   );
void MoveTo         (int r, int c);

void ResetCursorToPrompt();
void StartDebug(int max_cols);

class NrIO
{
public:
  NrIO(NrUI *nrui, int max_length, int prompt_height);
  ~NrIO();

  void ReadString();

  void SetMaxLength(int max_length) { this->max_length = max_length; };
  void SetPromptHeight(int prompt_height)
  {
    this->prompt_height = prompt_height;
  };

  const char *ToString() { return io_line.ToString(); };
  NrString GetIoLine()   { return io_line;            };

protected:

private:
  void Echo(size_t line_length);

  int  Getch  (     );
  bool IsAlpha(int c);
  bool IsNum  (int c);
  bool IsPunct(int c);
  bool IsCtrl (int c);

  NrUI *nrui;

  NrString io_line;
  int      max_length;
  int      prompt_height;
  size_t   cursor_position;
};

#endif
