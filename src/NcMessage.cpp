#include "NcMessage.h"

NcMessage::NcMessage()
{
  type = MSG_NONE;
  from[0] = '*';
  from[1] = '\0';
  to[0] = '*';
  to[1] = '\0';
  body[0] = '*';
  body[1] = '\0';
}

NcMessage::~NcMessage() {}

void NcMessage::Parse(const char *message_string, size_t length,
                      const char *username)
{
  size_t start_uname, end_uname;
  size_t start_body;

  if (message_string[0] == '<')
  {
    type = MSG_PRIVATE_RECEIVED;
    start_uname = 0;

    const char *result = strstr(message_string, " (private):");

    if (result != NULL)
    {
      end_uname = result - message_string;

      for (size_t i = start_uname; i < end_uname; i++)
      {
        from[i - start_uname] = message_string[i];
      }

      from[end_uname - start_uname] = '\0';

      for (size_t i = end_uname + 12; i < length; i++)
      {
        body[i - end_uname - 12] = message_string[i];
      }

      body[length - end_uname - 12] = '\0';
    }
  }

  else if ((strstr(message_string, ">> Message sent to")) != NULL)
  {
    type = MSG_PRIVATE_SENT;

    const char *result_start = strstr(message_string, "[");

    if (result_start != NULL)
    {
      const char *result_end = strstr(message_string, ":");

      if (result_end != NULL)
      {
        start_uname = result_start - message_string;
        end_uname = result_end - message_string;

        for (size_t i = start_uname; i < end_uname; i++)
        {
          to[i - start_uname] = message_string[i];
        }

        to[end_uname - start_uname] = '\0';
      }
    }

    const char *result = strstr(message_string, " (private):");

    if (result != NULL)
    {
      start_body = result - message_string + 12;

      for (size_t i = start_body; i < length; i++)
      {
        body[i - start_body] = message_string[i];
      }

      body[length - start_body] = '\0';
    }
  }

  else if (message_string[0] == '(')
  {
    type = MSG_EMOTE;
    strcpy(body, message_string);
  }

  else if (message_string[0] == '[')
  {
    const char *result_start = strstr(message_string, "]");
    char uname[MAX_NAME + 1];

    if (result_start != NULL)
    {
      const char *result_end = strstr(message_string, ":");

      if (result_end != NULL)
      {
        start_uname = result_start - message_string + 1;
        end_uname = result_end - message_string;

        for (size_t i = 0; i < end_uname; i++)
        {
          from[i] = message_string[i];
        }

        for (size_t i = start_uname; i < end_uname; i++)
        {
          uname[i - start_uname] = message_string[i];
        }

        from[end_uname] = '\0';
        uname[end_uname - start_uname] = '\0';

        if (strcmp(uname, username) == 0)
        {
          type = MSG_SENT;
        }

        else
        {
          type = MSG_RECEIVED;
        }

        start_body = end_uname + 2;

        for (size_t i = start_body; i < length; i++)
        {
          body[i - start_body] = message_string[i];
        }

        body[length - start_body] = '\0';
      }

      else
      {
        type = MSG_NONE;
        strcpy(body, message_string);
      }
    }
  }

  else if (message_string[0] == '>' && message_string[1] == '>')
  {
    type = MSG_SYSTEM;
    strcpy(body, message_string);
  }

  else
  {
    type = MSG_NONE;
    strcpy(body, message_string);
  }
}

void NcMessage::SetMessage(NcMessage *msg)
{
  type = msg->type;
  strcpy(from, msg->from);
  strcpy(to, msg->to);
  strcpy(body, msg->body);
}
