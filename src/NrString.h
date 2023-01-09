/*
 * File:   NrString.h
 * Author: Michel Alwan
 *
 * Created on 2022-12-31
 */

#ifndef NRSTRING_H
#define NRSTRING_H

#include "NrInclude.h"

class NrString
{
public:
  NrString();
  NrString(char c);
  NrString(const char *s);
  NrString(const NrString &nrs);
  ~NrString();

  NrString &operator=(const NrString &nrs);
  NrString operator+(const NrString &nrs) const;
  bool operator==(const NrString &nrs) const;
  void Echo();
  void Dump();
  void Clean();
  bool InsertCharAt(char c, size_t index);
  void DeleteCharAt(size_t index);
  NrString SubString(size_t start, size_t end) const;

  const char* ToString() const { return string; };
  const size_t GetLength()     { return length; };
  char GetCharAt(size_t index);
  int HaveChar(char c);

protected:

private:
  bool IsWhitespace(char c);
  
  char *string;
  size_t length;
};

#endif
