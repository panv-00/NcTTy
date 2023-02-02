/*
 * File:   NrScreen.cpp
 * Author: Michel Alwan
 *
 * Created on 2023-01-30
 */

#include "NrScreen.h"

NrScreen::NrScreen()
{

}

NrScreen::~NrScreen()
{

}

void NrScreen::ClrScr()
{
  printf("\033c");
  HideCursor();
}

void NrScreen::SavePosition()
{
  printf("\x1b%d", 7);
}

void NrScreen::RestorePosition()
{
  printf("\x1b%d", 8);
}

void NrScreen::SetColor(ColorCode cc)
{
  printf("\x1b[%dm", cc);
}

void NrScreen::MoveUp(int count)
{
  printf("\x1b[%dA", count);
}

void NrScreen::MoveDown(int count)
{
  printf("\x1b[%dB", count);
}

void NrScreen::MoveRight(int count)
{
  printf("\x1b[%dC", count);
}

void NrScreen::MoveLeft(int count)
{
  printf("\x1b[%dD", count);
}

void NrScreen::MoveTo(int r, int c)
{
  printf("\x1b[%d;%df", r, c);
}

void NrScreen::ScrollUp()
{
  //printf("\033[S");
}

void NrScreen::ScrollDn()
{
  //printf("\033[T");
}

void NrScreen::HideCursor()
{
  printf("\033[?25l");
}

void NrScreen::ShowCursor()
{
  printf("\033[?25h");
}
