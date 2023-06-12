#ifndef NCMESSAGE_H
#define NCMESSAGE_H

#define MAX_MESSAGE_BODY 4096
#define MAX_NAME 17

#include <stddef.h>
#include <string.h>

typedef enum
{
  MSG_NONE = 0,
  MSG_SENT,
  MSG_RECEIVED,
  MSG_PRIVATE_SENT,
  MSG_PRIVATE_RECEIVED,
  MSG_EMOTE,
  MSG_SYSTEM

} MessageType;

class NcMessage
{
public:
  NcMessage();
  ~NcMessage();

  void Parse(const char *message_string, size_t length, const char *username);
  void SetMessage(NcMessage *msg);

  MessageType GetType() { return type; };
  char *GetFrom() { return from; };
  char *GetTo() { return to; };
  char *GetBody() { return body; };

private:
  MessageType type;
  char from[MAX_NAME + 1];
  char to[MAX_NAME + 1];
  char body[MAX_MESSAGE_BODY + 1];
};

#endif
