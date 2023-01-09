/*
 * File:   NrString.cpp
 * Author: Michel Alwan
 *
 * Created on 2022-12-31
 */

#include "NrString.h"

/*
 * Public Functions
 */

NrString::NrString()
{
  length = 0;
  string = new char[length + 1];
  string[length] = '\0';
}

NrString::NrString(char c)
{
  length = 1;
  string = new char[length + 1];
  string[0] = c;
  string[length] = '\0';
}

NrString::NrString(const char *s)
{
  if (s == NULL)
  {
    length = 0;
    string = new char[1];
    string[0] = '\0';
  }

  else
  {
    length = 0;
    const char *str = s;

    while (*str != '\0') { length++; str++; }
    if (length > BUFSIZE) { length = BUFSIZE; }

    string = new char[length + 1];

    for (size_t i = 0; i < length; i++) { string[i] = s[i]; }

    string[length] = '\0';
  }
}

NrString::NrString(const NrString &nrs)
{
  length = nrs.length;
  string = new char[length + 1];

  for (size_t i = 0; i < length; i++) { string[i] = nrs.string[i]; }
  
  string[length] = '\0';
}

NrString::~NrString()
{
  delete[] string;
}

NrString &NrString::operator=(const NrString &nrs)
{
  if (this != &nrs)
  {
    delete[] string;
    length = nrs.length;
    string = new char[length + 1];
    
    for (size_t i = 0; i < length; i++)
    {
      string[i] = nrs.string[i];
    }

    string[length] = '\0';
  }

  return *this;
}

NrString NrString::operator+(const NrString &nrs) const
{
  NrString result;
  
  result.length = length + nrs.length;

  if (result.length > BUFSIZE) { result.length = BUFSIZE; }

  delete[] result.string;

  result.string = new char[result.length + 1];
  
  for (size_t i = 0; i < length; i++)
  {
    result.string[i] = string[i];
  }

  for (size_t i = 0; i < result.length - length; i++)
  {
    result.string[i + length] = nrs.string[i];
  }

  result.string[result.length] = '\0';

  return result;
}

bool NrString::operator==(const NrString &nrs) const
{
  if (length != nrs.length) { return false; }

  for (size_t i = 0; i < length; i++)
  {
    if (string[i] != nrs.string[i]) { return false; }
  }

  return true;
}

void NrString::Echo()
{
  printf("%s", string);
}

void NrString::Dump()
{
  int c;
  size_t count = 0;
  printf("Address = %p\n", &string[0]);
  while (true)
  {
    c = string[count++];
    if (c == '\0')
    {
      printf("(%4ld) - '\\0'\n", count);
      printf("Counted: %4ld chars. Defined: %4ld chars.\n\n", count - 1, length);
      break;
    }
    
    printf("(%4ld) - '%c'\n", count, c);
  }
}

void NrString::Clean()
{

  size_t start = 0;
  while (IsWhitespace(string[start]))
  {
    start++;
    if (start == length - 1) { break; }
  }
 
  size_t end = 0;
  if (length >= 1) { end = length - 1; }
  while (IsWhitespace(string[end]))
  {
    end--;
    if (end == 0) { break; }
  }
  
  if (start > end + 1)
  {
    length = 0;
    char *new_string = new char[1];
    new_string[0] = '\0';
    if (string != NULL) delete[] string;
    string = new_string;
  }

  else
  {
    size_t new_length = end - start + 1;
    char *new_string = new char[new_length + 1];

    for (size_t i = 0; i < new_length; i++) {
      new_string[i] = string[start + i];
    }

    new_string[new_length] = '\0';
    delete[] string;
    string = new_string;
    length = new_length;
  }
}

bool NrString::InsertCharAt(char c, size_t index)
{
  if (index > length || index < 0) { return false; }

  length++;

  if (length > BUFSIZE) { length = BUFSIZE; }

  char *new_string = new char[length + 1];

  for (size_t i = 0; i < index; i++) { new_string[i] = string[i]; }

  new_string[index] = c;

  for (size_t i = index + 1; i < length; i++)
  {
    new_string[i] = string[i - 1];
  }

  new_string[length] = '\0';
  delete[] string;

  string = new_string;

  return true;
}

void NrString::DeleteCharAt(size_t index)
{
  if (index >= length || index < 0) { return; }

  char *new_string = new char[length];

  for (size_t i = 0; i < index; i++) { new_string[i] = string[i]; }
  for (size_t i = index; i < length - 1; i++)
  {
    new_string[i] = string[i + 1];
  }

  new_string[length - 1] = '\0';
  delete[] string;

  string = new_string;
  length--;
}

NrString NrString::SubString(size_t start, size_t end) const
{
  if (start > end || end >= length)
  {
    return NrString();
  }

  size_t sub_length = end - start + 1;
  char *sub_string = new char[sub_length + 1];

  for (size_t i = 0; i < sub_length; i++)
  {
    sub_string[i] = string[start + i];
  }

  sub_string[sub_length] = '\0';

  NrString sub(sub_string);
  delete[] sub_string;
  
  return sub;
}


char NrString::GetCharAt(size_t index)
{
  if (index >= length || index < 0) { return '\0'; }

  return string[index];
}

int NrString::HaveChar(char c)
{
  for (int i = 0; i < (int) length; i++)
  {
    if (string[i] == c) { return i; }
  }

  return -1;
}

/*
 * Private Functions
 */

bool NrString::IsWhitespace(char c)
{
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}
