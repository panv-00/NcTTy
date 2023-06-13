#include "NcTTyUi.h"

int main(int argc, char *argv[])
{
  NcTTyUi *ui = new NcTTyUi();

  ui->Run();

  delete ui;

  return 0;
}
