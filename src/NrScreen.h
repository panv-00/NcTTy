/*
 * File:   NrScreen.h
 * Author: Michel Alwan
 *
 * Created on 2023-01-30
 */

#ifndef NRSCREEN_H
#define NRSCREEN_H

#include "NrInclude.h"

class NrScreen
{
public:
  NrScreen();
  ~NrScreen();

  void ClrScr         (            );
  void SavePosition   (            );
  void RestorePosition(            );
  void SetColor       (ColorCode cc);
  void MoveUp         (int count   );
  void MoveDown       (int count   );
  void MoveRight      (int count   );
  void MoveLeft       (int count   );
  void MoveTo         (int r, int c);
  void ScrollUp       (            );
  void ScrollDn       (            );
  void HideCursor     (            );
  void ShowCursor     (            );
protected:

private:

};

#endif
