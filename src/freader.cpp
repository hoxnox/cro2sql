/******************************************************************************
ProjectName: cro2sql
FileName: freader.cpp 20081203
Subj: "Докрутка" fstream для возможности пролиcтывать файл до определенного символа.
Author: Nosov Yuri
*******************************************************************************
  (c) Copyright 2008 Nosov Yuri (cro2sql@gmail.ru)

  The contents of this file are subject to the CPSQL License (the "License");
  you may not use this file except in compliance with the License.  You may
  obtain a copy of the License in the 'LICENSE' file which must have been
  distributed along with this file.

  This software, distributed under the License, is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
  License for the specific language governing rights and limitations
  under the License.
******************************************************************************/
#include "freader.h"
#include <cstring>
#include <stdexcept>

using namespace std;

CFileReader::CFileReader() : ifstream()
{

}

CFileReader::CFileReader(string path) : ifstream(path.c_str())
{

}

bool CFileReader::skip_until(const string &delim, char end_symbol /*=0*/)
{
  size_t delim_len=delim.length();
  string buffer;
  char current;
  for(size_t i = 0; i < delim_len; i++)
  {
    get(current);
    if(eof() || current == end_symbol)
    {
      return false;
    }
    buffer += current;
  }
  if( buffer == delim )
    return true;
  for(get(current); gcount() > 0 && !eof() && current != end_symbol; get(current))
  {
    for(size_t j = 0; j < delim_len - 1; j++)
      buffer[j] = buffer[j+1];
    buffer[delim_len - 1] = current;
    if( buffer == delim )
      return true;
  }
  return false;
}

bool CFileReader::get_is_equal(const string &str, char end_symbol /*=0*/)
{
  size_t str_len = str.length();
  if( peek() == end_symbol || peek() != str[0] )
    return false;
  char * tmp = new char[str_len+1];
  if( !tmp )
    throw runtime_error("Memory error");
  get(tmp, str_len+1);
  if( (size_t)gcount() <  str_len || eof())
  {
    delete [] tmp;
    return false;
  }
  if( strncmp(str.c_str(), tmp, str_len) == 0)
  {
    delete [] tmp;
    return true;
  }
  else
  {
    delete [] tmp;
    return false;
  }
}

string CFileReader::get_until(const string &delim, char end_symbol /* = 0 */)
{
  size_t delim_len=delim.length();
  string result = "";
  string buffer;
  char current = 0;
  for(size_t i = 0; i < delim_len; i++)
  {
    get(current);
    if(eof() || current == end_symbol)
    {
      return buffer;
    }
    buffer += current;
  }
   if( buffer == delim )
     return "";
   result = buffer;
   for(get(current); gcount() > 0 && !eof() && current != end_symbol; get(current))
   {
     for(size_t j = 0; j < delim_len - 1; j++)
       buffer[j] = buffer[j+1];
     buffer[delim_len - 1] = current;
     if( buffer == delim )
       return result.substr(0, result.length() - delim_len + 1);
     result += current;
   }
   return result;
}
