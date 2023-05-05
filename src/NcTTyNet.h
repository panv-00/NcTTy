/*
 * File:   NcTTyNet.h
 * Created on 2023-01-31
 */

#ifndef NCTTYNET_H
#define NCTTYNET_H

#include "NcTTyUi.h"

#include <arpa/inet.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>

#define ADDRESS "76.235.94.33"
// #define ADDRESS "127.0.0.1"
#define PORT 6667
#define BUFSIZE 2048

typedef enum
{
  NO_NET_ERROR = 0,
  SOCKET_ERROR,
  SERVER_ERROR,
  SSL_ERROR,
  ERROR_RECEIVING_DATA,
  CONNECTION_CLOSED

} NetworkStatus;

const char *NetStatusToText(NetworkStatus code);

class NcTTyNet
{
public:
  NcTTyNet();
  ~NcTTyNet();

  NetworkStatus Connect();
  void End();
  NetworkStatus ReceiveData(NcTTyUi *ui);

  void StopReceiving() { stop_receiving = true; };
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
