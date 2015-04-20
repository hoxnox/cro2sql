/******************************************************************************
ProjectName: cro2sql
FileName: mian.cpp
Subj: Enter point
Author: Nosov Yuri
*******************************************************************************
(c) Copyright 2008 Nosov Yuri (cro2sql@gmail.ru)

The contents of this file are subject to the cro2sql License (the "License");
you may not use this file except in compliance with the License.  You may
obtain a copy of the License in the 'LICENSE' file which must have been
distributed along with this file.

This software, distributed under the License, is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
License for the specific language governing rights and limitations
under the License.
******************************************************************************/

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif // EXIT_SUCCESS
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif // EXIT_FAILURE
#ifndef EXIT_WARNING
#define EXIT_WARNING 2
#endif // EXIT_WARNING


#include "getopt.hpp"
#include "messages.hpp"
#include "dbstruct.h"
#include "misc.h"
#include "dataparser.h"


int main(int argc, const char * argv[])
{

  string structure_path = "";
  string bank_name = "";
  string tune_file = "";
  size_t extended_lines = 0;
  string export_path = "";
  string sql_path = "";
  string data_separator = "|";
  bool silent = false;

  // Обрабатываем пользовательские опции и входные данные (к 19.01.209)
  COptionsManager * OptionManager = COptionsManager::Instance();
  OptionManager->register_option('c', "config", COptionsManager::REQUIRED_ARGUMENT);
  OptionManager->register_option('d', "delimiter", COptionsManager::REQUIRED_ARGUMENT);
  OptionManager->register_option('e', "export", COptionsManager::REQUIRED_ARGUMENT);
  OptionManager->register_option('o', "output", COptionsManager::REQUIRED_ARGUMENT);
  OptionManager->register_option('m', "extended", COptionsManager::OPTIONAL_ARGUMENT);
  OptionManager->register_option('t', "tune", COptionsManager::REQUIRED_ARGUMENT);
  OptionManager->register_option('s', "silent", &silent);
  OptionManager->register_option('h', "help", COptionsManager::NO_ARGUMENT);
  for(int result = OptionManager->getopt(argc, argv); result != -1; result = OptionManager->getopt(argc, argv))
  {
    string tmp = "";
    size_t ind;
    switch(result)
    {
    case ':':
      cerr << GetErrorMsg(E_WRONG_ARG) << endl;
      return EXIT_FAILURE;
    case '?':
      if( !export_path.empty() ){
        cerr << GetErrorMsg(E_UNKNOWN_OPT) << endl;
        return EXIT_FAILURE;
      }
      export_path = argv[OptionManager->cursor()];
    break;
    case 1:
      switch(OptionManager->current_short_name())
      {
      case 'h':
        if(!silent)
          cout << GetFullHelp() << endl;
        return EXIT_SUCCESS;
      case 'c':
        tmp = OptionManager->current_arguments();
        ind = tmp.find(";");
        if(ind == string::npos)
        {
          structure_path = tmp;
          bank_name = "";
        }
        else
        {
          structure_path = tmp.substr(0, ind);
          ind++;
          bank_name = tmp.substr(ind, tmp.length() - ind);
        }
        break;
      case 'e':
       export_path = OptionManager->current_arguments();
       break;
      case 'm':
        extended_lines = atoi(OptionManager->current_arguments().c_str());
        if( errno == ERANGE || extended_lines == 0 || atoi(OptionManager->current_arguments().c_str()) < 0)
        {
          cerr << GetErrorMsg(E_EXT_LINES_RANGE) << endl;
          return EXIT_FAILURE;
        }
        break;
      case 't':
        tune_file = OptionManager->current_arguments();
        break;
      case 'o':
        sql_path = OptionManager->current_arguments();
        break;
      case 'd':
        data_separator = OptionManager->current_arguments();
        break;
      }
    break;
    }
  }
  if( !silent )


  // Все переменные должны быть инициализированы после разбора опций
  if( data_separator.empty() || export_path.empty() || structure_path.empty() || sql_path.empty())
  {
    cerr << GetErrorMsg(E_OPT) << endl;
    return EXIT_FAILURE;
  }

  size_t max_tab_ident = 20;
  size_t max_field_ident = 30;
  size_t max_varchar_size = 4000;
  string delimiter_escaper = "'";
  string delimiter_must_be_esc = "'";
  string int_type = "INTEGER";
  string long_type = "LONG";
  string float_type = "FLOAT";
  string clob_type = "TEXT";
  string varchar_type = "VARCHAR";
  string date_type  = "DATE";
  string time_type = "TIME";

  // Проверяем правильность путей файлов
  if( !(file_mode( structure_path ) & S_IFREG) || !(file_mode( sql_path ) & S_IFDIR) || !(file_mode( export_path ) & S_IFDIR) || (!sql_path.empty() && !(file_mode( sql_path ) & S_IFDIR)) )
  {
    cerr << GetErrorMsg(E_FILE_NOT_FOUND) << endl;
    return EXIT_FAILURE;
  }
  if( !tune_file.empty() )
  {
    if( !(file_mode( tune_file ) & S_IFREG) )
    {
      cerr << GetErrorMsg(E_FILE_NOT_FOUND) << endl;
      return EXIT_FAILURE;
    }
    ifstream ftune(tune_file.c_str());
    if( ftune.bad() )
    {
      cerr << GetErrorMsg(E_FILE_OPEN_ERROR) << endl;
      return EXIT_FAILURE;
    }
    ftune >> max_tab_ident;
    ftune >> max_field_ident;
    ftune >> max_varchar_size;
    ftune >> delimiter_escaper;
    ftune >> delimiter_must_be_esc;
    ftune >> int_type;
    ftune >> long_type;
    ftune >> float_type;
    ftune >> clob_type;
    ftune >> varchar_type;
    ftune >> date_type;
    ftune >> time_type;
    if( errno == ERANGE)
    {
      cerr << GetErrorMsg(E_TUNE_FILE) << endl;
      return EXIT_FAILURE;
    }
    ftune.close();
  }

  CDatabase db(max_tab_ident, max_field_ident, max_varchar_size, delimiter_escaper.c_str(), delimiter_must_be_esc.c_str(), int_type.c_str(), long_type.c_str(), float_type.c_str(), clob_type.c_str(), varchar_type.c_str(), date_type.c_str(), time_type.c_str());
  db.separator = data_separator;
  db.name = bank_name;
  db.data_folder = export_path;

  int result = db.read_from_file(structure_path, bank_name);
  if(  result == EXIT_FAILURE)
  {
    cerr << GetErrorMsg( E_PARSE_ERROR + CP1251_CP866(db.get_last_error())) << endl;
    return EXIT_FAILURE;
  }
  if( result == EXIT_WARNING )
  {
    if(!silent)
    {
      if(!silent)
        cout << W_PREFIX << " " <<  CP1251_CP866(db.get_warnings());
    }
  }

  CDataParser data_parser(&db);
  for(vector<SDataFile>::iterator i = data_parser.data_files.begin(); i != data_parser.data_files.end(); i++)
  {
    ofstream fw;
    if( i->table != NULL )
    {
      fw.open((sql_path + "/" + i->table->sql_name + ".sql").c_str());
      if(fw.bad())
      {
        cerr << GetErrorMsg( (string)W_PREFIX + "Невозможно открыть файл \""+ i->table->sql_name +"\".") << endl;
        continue;
      }
    }
    if( i->reference != NULL )
    {
      fw.open((sql_path + "/" + i->reference->sql_name + ".sql").c_str());
      if(fw.bad())
      {
        cerr << GetErrorMsg( (string)W_PREFIX + "Невозможно открыть файл \""+ i->reference->sql_name +"\".") << endl;
        continue;
      }
    }
    data_parser.use_file(i);
    string header = "";

    if( extended_lines == 0 )
    {
      while(data_parser.get_status() != data_parser.END)
      {
        string tmp;
        if(data_parser.get_status() == data_parser.BEGIN)
        {
          header = data_parser.parse_next_line();
          if( header.empty() && !(data_parser.get_last_error()).empty() )
          {
            cerr << GetErrorMsg( E_DATA_PARSE + CP1251_CP866(data_parser.get_last_error())) << endl;
            return EXIT_FAILURE;
          }
          continue;
        }
        tmp = data_parser.parse_next_line();
        if( !(data_parser.get_warnings()).empty() )
        {
          if(!silent)
            cout << W_PREFIX << endl <<  CP1251_CP866(data_parser.get_warnings());
          data_parser.clear_warnings();
        }
        if( tmp.empty() )
        {
          if( data_parser.get_warning_type() != data_parser.NO_WARNINGS)
            continue;
          if( !(data_parser.get_last_error()).empty() )
          {
            cerr << GetErrorMsg( E_DATA_PARSE + CP1251_CP866(data_parser.get_last_error())) << endl;
            return EXIT_FAILURE;
          }
        }
        else
          fw << header + tmp + ";" << endl;
      }
    }
    else
    {
      size_t counter = 0;
      bool was_header = false;
      while(data_parser.get_status() != data_parser.END)
      {
        string tmp;
        if( counter == extended_lines )
        {
          counter = 0;
          fw << ";" << endl << endl << header << "\\" << endl;
          was_header = true;
        }
        if(data_parser.get_status() == data_parser.BEGIN)
        {
          header = data_parser.parse_next_line();
          if( header.empty() && !(data_parser.get_last_error()).empty() )
          {
            cerr << GetErrorMsg( E_DATA_PARSE + CP1251_CP866(data_parser.get_last_error())) << endl;
            return EXIT_FAILURE;
          }
          fw << header << "" << endl;
          counter++;
          was_header = true;
          continue;
        }
        tmp = data_parser.parse_next_line();
        if( !(data_parser.get_warnings()).empty() )
        {
          if(!silent)
            cout << W_PREFIX << endl <<  CP1251_CP866(data_parser.get_warnings());
          data_parser.clear_warnings();
        }
        if( tmp.empty() )
        {
          if( data_parser.get_warning_type() != data_parser.NO_WARNINGS)
            continue;
          if( !(data_parser.get_last_error()).empty() )
          {
            cerr << GetErrorMsg( E_DATA_PARSE + CP1251_CP866(data_parser.get_last_error())) << endl;
            return EXIT_FAILURE;
          }
          else
          {
            fw << ";" << endl;
            was_header = false;
          }
        }
        else
        {
          if( !was_header )
            fw << "," << endl;
          else
            was_header = false;
        }
        fw << tmp;
        counter++;
      }
    }
    fw.close();
  }
  ofstream ddl_file((sql_path + "/index-ddl.sql").c_str());
  if( ddl_file.bad() )
    cerr << GetErrorMsg( (string)W_PREFIX + "Невозможно открыть файл \""+ sql_path + "/index-ddl.sql" +"\".") << endl;
  else
    ddl_file << db.generate_DDL();
  ddl_file.close();
  //getchar();
  return EXIT_FAILURE;
}

/***************************************/
