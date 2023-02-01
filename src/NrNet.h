/*
 * File:   NrNet.h
 * Author: Michel Alwan
 *
 * Created on 2023-01-31
 */

#ifndef NRNET_H
#define NRNET_H

#include "NrInclude.h"
#include "NrUI.h"

const char *NetStatusToText(NetworkStatus code);

class NrNet
{
public:
  NrNet();
  ~NrNet();

  NetworkStatus Connect();
  void End();
  NetworkStatus ReceiveData(NrUI *ui);

  void StopReceiving()     { stop_receiving = true;     };
  bool FinishedReceiving() { return finished_receiving; };

  SSL *GetSSL() { return ssl; };

protected:

private:
  bool connected;
  int sockfd;
  struct sockaddr_in serv_addr;
  SSL_CTX *ctx;
  SSL *ssl;

  bool stop_receiving;
  bool finished_receiving;
};

#endif
