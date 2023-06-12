/*
 * File:   NcTTy.cpp
 * Created on 2023-04-17
 */

#include "NcTTyUi.h"

int main(int argc, char *argv[])
{
  NcTTyUi *ui = new NcTTyUi();

  ui->Run();

  delete ui;

  return 0;
}
