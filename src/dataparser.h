/******************************************************************************
ProjectName: cro2sql
FileName: dataparser.h 20080110
Subj: Определяет класс разборщика файлов-данных
Author: Nosov Yuri
*******************************************************************************
  (c) Copyright 2008 Nosov Yuri (cro2sql@gmail.com)

  The contents of this file are subject to the CPSQL License (the "License");
  you may not use this file except in compliance with the License.  You may
  obtain a copy of the License in the 'LICENSE' file which must have been
  distributed along with this file.

  This software, distributed under the License, is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
  License for the specific language governing rights and limitations
  under the License.
******************************************************************************/

#ifndef __DATAPARSER_H__
#define __DATAPARSER_H__

#include "dbstruct.h"
#include "freader.h"

#include <string>


#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif // EXIT_SUCCESS
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif // EXIT_FAILURE
#ifndef EXIT_WARNING
#define EXIT_WARNING 2
#endif // EXIT_WARNING

typedef struct
{
  CTable * table;
  CReference * reference;
  std::string file_path;
  std::vector<CField *> fields_sequence;
} SDataFile;

class CDataParser
{
public:
  typedef enum{
    BEGIN,
    PROCESS,
    END
  } TParceStatus;
  typedef enum{
    NO_WARNINGS,
    EMPTY_LINE,
    WRONG_STRING,
    WRONG_HEADER,
    WRONG_TYPE
  } TWarnings;
  CDataParser(CDatabase *);
  ~CDataParser();
  std::string get_last_error() const {return error_msg;};
  std::string get_warnings();
  TParceStatus get_status() const {return _parse_status;};
  TWarnings get_warning_type() const {return _warning_type;};
  void clear_warnings();
  int use_file(std::vector<SDataFile>::iterator data_file);
  std::string parse_next_line();
  // Возвращает тип данных, которому может соответствовать данная строка
  std::string get_data_type(std::string data);
  std::string fetch_field_data(std::string, CField *);
  // Основные типы данных конкретной реализации SQL
  std::vector<SDataFile> data_files;
  double processed_size;
  double data_size;


protected:
  std::string error_msg;
  std::vector<std::string> warning_msgs;
  std::vector<SDataFile>::iterator current_file;
  CFileReader * fr;
  CDatabase * db;
  TParceStatus _parse_status;
  TWarnings _warning_type;
};


#endif // __DATAPARSER_H__
