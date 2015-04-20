/******************************************************************************
ProjectName: getopt
FileName: getopt.cpp 2008.12.01
Subj: Options parsing
Author: Nosov Yuri
*******************************************************************************
(C) Copyright 2008 Nosov Yuri (cro2sql@mail.ru)

  The contents of this file are subject to the License (the "License");
  you may not use this file except in compliance with the License.  You may
  obtain a copy of the License in the 'LICENSE' file which must have been
  distributed along with this file.

  This software, distributed under the License, is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
  License for the specific language governing rights and limitations
  under the License.
******************************************************************************/

#include "getopt.hpp"
#include <algorithm>

using namespace std;

getopt_error::getopt_error(string str) : logic_error(str)
{ }
registration_error::registration_error(string str) : getopt_error(str)
{ }
parce_error::parce_error(string str) : getopt_error(str)
{ }

COptionsManager::COptionsManager() :_arguments(""),  _posix_W(false), _cursor(0), _reminder(""), _all_short_names("")
{ }

void COptionsManager::set_posix_W()
{
  if( !_posix_W )
  {
    _posix_W = true;
    _all_short_names += 'W';
  }
}

// Initializing static variable by 0
COptionsManager * COptionsManager::_instance = 0;

COptionsManager * COptionsManager::Instance()
{
  if( _instance == 0)
    _instance = new COptionsManager;
  return _instance;
}

inline bool _is_alpha(const char a)
{
  if( ('a' <= a && a <= 'z') || ('A' <= a && a <= 'Z') )
    return true;
  else
    return false;
}

inline bool _is_digit(const char a)
{
  if( '0' <= a && a <= '9' )
    return true;
  else
    return false;
}

bool COptionsManager::is_registered(string long_name)
{
  vector<string>::iterator result = find(_all_long_names.begin(), _all_long_names.end(), long_name);
  if  ( result != _all_long_names.end( ) )
    return true;
  return false;
}

bool COptionsManager::is_registered(char short_name)
{
  if(_all_short_names.find(short_name) != string::npos)
    return true;
  return false;
}

int COptionsManager::cursor() const
{
  return _cursor;
}

inline bool _is_valid_symbol(char symbol)
{
  if( !_is_alpha(symbol) && !_is_digit(symbol) && symbol != '_' )
    return false;
  return true;
}

bool COptionsManager::_test_long_optname(string str)
{
  if( str.length() >= 1 && _is_alpha(str[0]) )
  {
    for(size_t i = 1; i < str.length(); i++)
      if( !_is_valid_symbol(str[i]) )
        return false;
  }
  else
    return false;
  return true;
}

// тестирует имена опций и, если все ок, заполняет option и добаляет имена в индексы поиска
void COptionsManager::_test_set_names(const char short_name, const string &long_name, const string &description, TOption &option)
{
  /* SHORT_NAME PROCESSING */
  // test short_name
  if(short_name != ' ')
  {
    if( !_is_alpha(short_name) && short_name != ' ' )
      throw registration_error((string)"Option name \"" + (string)&short_name + "\" is wrong. It must be only latin symbols.");
    //unique
    if( is_registered(short_name) )
      throw registration_error( "Option name \"" + (string)&short_name + "\" is not unique." );
    // tests ok - writing short name
    option.short_name = short_name;
    _all_short_names += short_name;
  }
  /**/

  /* LONG_NAME PROCESSING */
  // test long_name
  if( long_name != "" )
  {
    if( !_test_long_optname(long_name) )
      throw registration_error("Option name \"" + long_name + "\" is wrong. It mus be a string, containing only latin symbols, digits and \"_\" symbol. Latin symbol must be first.");
    //unique
    if( is_registered(long_name) )
      throw registration_error( "Option name \"" + long_name + "\" is not unique." );
    //tests ok - writing long name
    option.long_name = long_name;
    _all_long_names.push_back(long_name);
    /**/
  }
  else
  {
    if( short_name == ' ' )
      throw registration_error("Long and Short options can't be empty both.");
    option.long_name.clear();
  }

  if( description == "" )
    option.description.clear();
  else
    option.description = description;
}

void COptionsManager::register_option(char short_name, string long_name, bool * flag, string description /* = "" */ )
{
  TOption option;
  _test_set_names(short_name, long_name, description, option);
  option.flag = flag;
  option.argument_type = NO_ARGUMENT;
  options.push_back(option);
}

void COptionsManager::register_option(char short_name, std::string long_name, COptionsManager::ARGTYPE argument_type, std::string description /* = "" */)
{
  TOption option;
  _test_set_names(short_name, long_name, description, option);
  option.argument_type = argument_type;
  option.flag = NULL;
  options.push_back(option);
}

char COptionsManager::current_short_name() const
{
  return option->short_name;
}

string COptionsManager::current_long_name() const
{
  return option->long_name;
}

string COptionsManager::current_arguments() const
{
  return _arguments;
}

int COptionsManager::getopt(int argc, const char * argv[])
{
  if( argc < 1 )
    throw parce_error("Command line must contain at least 1 argument.");
  if( argv[argc] != NULL )
    throw parce_error("Arguments count argc not submited with argv.");

  // Просмотрели ли все опции
  if(_cursor >= argc)
  {
    _reminder.clear();
    return -1;
  }
  if( _reminder.empty() )
  {
    _cursor++;
    if( argv[_cursor] == NULL )
    {
      _reminder.clear();
      return -1;
    }
    _reminder = argv[_cursor];
  }

  size_t length = _reminder.length();
  if( length == 0)
    throw parce_error("One of the argv elements is empty.");

  //Если длина аргумента меньше 2 или аргмент не начинается с "-", он не может быть опцией
  if( length < 2 || _reminder[0] != '-')
  {
    _reminder.clear();
    return '?';
  }

  // Кандидат на короткую опцию
  if( _is_alpha(_reminder[1]) && is_registered(_reminder[1]) && !(_posix_W && _reminder[1] == 'W'))
  {
    // Выставляем итератор option на соответствующую опцию
    for( option = options.begin(); option != options.end(); option++)
    {
      if( option->short_name == _reminder[1] )
      {
        if( option->argument_type == NO_ARGUMENT )
        {
          // Если у опции не должно быть аргументов, но после ее имени следует не имя опции, то ошибка в аргументах опции
          if ( length > 2 )
            if ( !is_registered(_reminder[2]) )
            {
              _reminder.clear();
              return ':';
            }
          // Если длина = 2 или длина > 2 и _reminder[2] - опция
          _reminder = _reminder.substr(0,1) + _reminder.substr(2, length);
          if(_reminder == "-")
            _reminder.clear();
          if( option->flag )
          {
            *option->flag = true;
            return 0;
          }
          else
            return 1;
     }
        else
        {
          if(length > 2)
          {
              _arguments = _reminder.substr(2, length);
            _reminder.clear();
            return 1;
          }
          if( argv[_cursor+1] == NULL || argv[_cursor+1][0] == '-' )
          {
            if( option->argument_type == REQUIRED_ARGUMENT )
            {
              _reminder.clear();
              return ':';
            }
            else
            {
              _reminder = _reminder.substr(0,1) + _reminder.substr(2,length);
              if( _reminder  == "-")
                _reminder.clear();
              return 1;
            }
          }
          else
          {
            // Следующий элемент argv[] - аргументы нашей опции, поэтому расширяем _reminder на argv[_cursor+1] и запускаем getopt заново. Он пойдет по тому же пути, что и этот (так как начало _reminder осталось неизменным, за исключением строки if(length > 2))
            _reminder += argv[_cursor+1];
            _cursor++;
            return getopt(argc, argv);
          }
        }
      }//for _cursor
    }
    // Если мы так и не нашли соответствующий объект вектора, то произошел какой-то сбой в is_registered, register_option или в цикле выше
    throw getopt_error("Internal error. Cursor for oprion \""+(string)&_reminder[1]+"\" was not set.");
  }
  // Кандидат на длинную опцию
  if( _reminder[1] == '-' || (_posix_W && _reminder[1] == 'W') )
  {
    //Если длина аргумента, начинающегося с "-", меньше 3 или третий символ - не буква, то данный аргумент не может быть опцией
    if( length < 3 || !_is_alpha(_reminder[2]) )
    {
      _reminder.clear();
      return '?';
    }
    size_t i = 3;
    for(; i < length; i++)
      if( !_is_valid_symbol(_reminder[i]) )
      {
        break;
      }

    string opt_candidate = _reminder.substr(2, i-2);
    if( !is_registered( opt_candidate ) )
    {
      _reminder.clear();
      return '?';
    }

    // выставляем курсор
    for( option = options.begin(); option != options.end(); option++)
    {
      if( option->long_name == opt_candidate )
      {
        if( option->argument_type == NO_ARGUMENT )
        {
          if( i < length )
          {
            _reminder.clear();
            return ':';
          }
          else
          {
            if( option->flag )
            {
              *option->flag = true;
              _reminder.clear();
              return 0;
            }
            else
            {
              _reminder.clear();
              return 1;
            }
          }
        }
        else
        {
          if( i < length && _reminder[i] != '=' )
          {
            _reminder.clear();
            return ':';
          }
          i++;
          if( i >= length )
          {
            if( option->argument_type == REQUIRED_ARGUMENT )
            {
              _reminder.clear();
              return ':';
            }
            else
            {
              if( option->flag )
              {
                *option->flag = true;
                _reminder.clear();
                return 0;
              }
              _reminder.clear();
              return 1;
            }
          }
          _arguments = _reminder.substr(i, length - i);
          _reminder.clear();
          return 1;
        }
      }
    }
    // Если мы так и не нашли соответствующий объект вектора, то произошел какой-то сбой в is_registered, register_option или в цикле выше
    throw getopt_error("Internal error. Cursor for oprion \""+(string)&_reminder[1]+"\" was not set.");
  }

  // Если символ, идущий за "-" - не буква и не символ "-", то аргумент недействительная опция
  _reminder.clear();
  return '?';
}

void COptionsManager::reset()
{
  for(option = options.begin(); option != options.end(); option++)
  {
    _arguments.clear();
  }
  _reminder.clear();
  _cursor = 0;
}
