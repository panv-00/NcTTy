/*
 * File:   NrNet.h
 * Author: Michel Alwan
 *
 * Created on 2023-01-01
 */

#ifndef NRNET_H
#define NRNET_H

#include "NrInclude.h"
#include "NrString.h"
#include "NrUI.h"

const char *NetStatusToText(NetworkStatus code);

class NrNet
{
public:
  NrNet();
  ~NrNet();

  NetworkStatus Connect();
  void End();
  NetworkStatus ReceiveData(NrUI *nrui);

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

  bool stop_receiving = false;
  bool finished_receiving;
};

#endif
