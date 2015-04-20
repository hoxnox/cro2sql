#include <stdexcept>
#include "misc.h"

using namespace std;

int file_mode(string path)
{
  struct stat buf;
  int result;
  // Get data associated with "crtstat.c":
  result = stat( path.c_str(), &buf );
  // Check if statistics are valid:
  if( result != 0 )
  {
    if (errno == ENOENT)
      return 0;
    throw runtime_error("File information retrieving problem.\n");
  }
  else
    return buf.st_mode;
}
size_t file_size(string path)
{
  struct stat buf;
  int result;
  // Get data associated with "crtstat.c":
  result = stat( path.c_str(), &buf );
  // Check if statistics are valid:
  if( result != 0 )
  {
    if (errno == ENOENT)
      return 0;
    throw runtime_error("File information retrieving problem.\n");
  }
  else
    return buf.st_size;
}

string CP1251_CP866(string str)
{
  string cp;
  for(size_t i=0; i < str.length(); i++)
  {
    if( 192 <= (unsigned char)str[i] && (unsigned char)str[i] <= 239 )
    {
      cp += (unsigned char)str[i] - 64;
      continue;
    }
    if( 240 <= (unsigned char)str[i] )
    {
      cp += (unsigned char)str[i] - 16;
      continue;
    }
    cp += str[i];
  }
  return cp;
}

bool is_printable(char symbol)
{
  if( (unsigned char)symbol >= 32 )
    return true;
  else
    return false;
}
bool is_alpha(char symbol)
{
  if( ( 'a' <= symbol && symbol <= 'z') || ( 'A' <= symbol && symbol <= 'Z') || ( 'à' <= symbol && symbol <= 'ÿ') || ( 'À' <= symbol && symbol <= 'ß') )
    return true;
  else
    return false;
}

bool is_lat_alpha(char symbol)
{
  if( ( 'a' <= symbol && symbol <= 'z') || ( 'A' <= symbol && symbol <= 'Z') )
    return true;
  else
    return false;
}

bool is_digit(char symbol)
{
  if( '0' <= symbol && symbol <= '9')
    return true;
  else
    return false;
}

bool is_alpha(string str)
{
  for(size_t i = 0; i < str.length(); i++)
    if( !is_alpha(str[i]) )
      return false;
  return true;
}

bool is_digit(string str)
{
  for(size_t i = 0; i < str.length(); i++)
    if( !is_digit(str[i]) )
      return false;
  return true;
}

bool is_printable(string str)
{
  for(size_t i = 0; i < str.length(); i++)
    if( !is_printable(str[i]) )
      return false;
  return true;
}

bool is_operand(string str)
{
  if( !is_lat_alpha(str[0]) )
    return false;
  for(size_t i = 1; i < str.length(); i++)
    if( !is_lat_alpha(str[i]) && str[i] != '_' && !is_digit(str[i]) )
      return false;
  return true;
}
