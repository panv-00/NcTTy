/*
 * File:   NcTTy.cpp
 * Created on 2023-04-17
 */

#include "NcTTyUi.h"
#include <stddef.h>

int main(int argc, char *argv[])
{
  NcTTyUi *ui = new NcTTyUi();

  ui->SetupApp();
  ui->Run();

  delete ui;
  return 0;
}
