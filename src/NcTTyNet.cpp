#include "NcTTyNet.h"

const char *NetStatusToText(NetworkStatus code)
{
  switch (code)
  {
  case NO_NET_ERROR:
    return "No Network Error!";
  case SOCKET_ERROR:
    return "Socket Error!";
  case SERVER_ERROR:
    return "Server Error!";
  case SSL_ERROR:
    return "SSL Error!";
  case ERROR_RECEIVING_DATA:
    return "Cannot Receive Data!";
  case CONNECTION_CLOSED:
    return "Connection closed by server!";
  default:
    return "Unexpected Error!";
  }
}

NcTTyNet::NcTTyNet()
    : connected{false}, sockfd{0}, ctx{0}, ssl{0}, stop_receiving{false}
{
}

NcTTyNet::~NcTTyNet() { End(); }

NetworkStatus NcTTyNet::Connect()
{
  SSL_library_init();
  SSL_load_error_strings();
  ctx = SSL_CTX_new(SSLv23_client_method());
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
  {
    return SOCKET_ERROR;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  inet_pton(AF_INET, ADDRESS, &serv_addr.sin_addr);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    return SERVER_ERROR;
  }

  ssl = SSL_new(ctx);
  SSL_set_fd(ssl, sockfd);

  if (SSL_connect(ssl) != 1)
  {
    return SSL_ERROR;
  }

  finished_receiving = false;
  connected = true;

  return NO_NET_ERROR;
}

void NcTTyNet::End()
{
  if (connected)
  {
    int result = SSL_shutdown(ssl);

    if (result == 0)
    {
      result = SSL_shutdown(ssl);
    }

    SSL_free(ssl);
  }

  close(sockfd);
  SSL_CTX_free(ctx);
}

NetworkStatus NcTTyNet::ReceiveData(NcTTyUi *ui)
{
  while (!stop_receiving)
  {
    int fd = SSL_get_rfd(ssl);
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    int result = select(fd + 1, &read_fds, NULL, NULL, &timeout);
    if (result < 0)
    {
      return ERROR_RECEIVING_DATA;
    }
    else if (result == 0)
    {
      // timeout...
    }
    else
    {
      char buffer[BUFSIZE];
      int bytes_received = SSL_read(ssl, buffer, sizeof(buffer));

      if (bytes_received < 0)
      {
        return ERROR_RECEIVING_DATA;
      }
      if (bytes_received == 0)
      {
        ui->EndConnection();
        return CONNECTION_CLOSED;
      }
      ui->AddBufferToMessages(buffer, bytes_received);
    }
  }

  finished_receiving = true;

  return NO_NET_ERROR;
}
