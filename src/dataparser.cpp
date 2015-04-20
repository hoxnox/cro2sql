/******************************************************************************
ProjectName: cro2sql
FileName: dataparser.cpp 20080110
Subj: Реализация класса разборщика файлов-данных
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

#include "dataparser.h"
#include "misc.h"
#include <limits>
#include <cstdlib>
#include <cerrno>
#include <sstream>
#include <stdexcept>

using namespace std;

string CDataParser::get_warnings()
{
  string result;
  for(vector<string>::iterator wrn = warning_msgs.begin(); wrn != warning_msgs.end(); wrn++)
    result += *wrn + "\n";
  return result;
}

void CDataParser::clear_warnings()
{
  warning_msgs.clear();
}

CDataParser::CDataParser(CDatabase * db) : processed_size(0), data_size(0)
{
  SDataFile file;
  vector<CTable *> new_tabs;
  // Список файлов, которые должны быть:
  for( vector<CTable *>::iterator i = db->tables.begin(); i != db->tables.end(); i++)
  {
    file.reference = NULL;
    file.table = *i;
    file.file_path = db->data_folder + "/" + (*i)->file;
    data_files.push_back( file );
    for(vector<CField *>::iterator j = (*i)->fields.begin(); j != (*i)->fields.end(); j++)
      if( (*j)->attrib.find("МН") != string::npos )
      {
        CTable * tab = new CTable();
        if( !tab )
          throw runtime_error("Неудалось выделить память под множественную таблицу.");
        tab->sql_name = "mv_" + (*i)->sql_name + "_" + (*j)->sql_name;
        tab->cro_name = "Множественное поле: " + (*j)->cro_name;
        tab->file = tab->sql_name + ".sql";
        CField * id_field, *val_field;
        id_field = new CField();
        val_field = new CField();
        if( !id_field || !val_field )
          throw runtime_error("Неудалось выделить память под поля множественной таблицы.");
        id_field->sql_name = "id";
        id_field->sql_type = db->TYPE_NUMBER;
        id_field->cro_name = "SN";
        id_field->cro_length = "10";
        id_field->cro_type = id_field->TYPE_DIGITS;
        id_field->number = "0";
        id_field->flexible = false;
        id_field->table = tab;
        val_field->sql_name = "value";
        val_field->sql_type = (*j)->sql_type;
        val_field->cro_name = "MULTIVALUE";
        val_field->cro_length = (*j)->cro_length;
        val_field->cro_type = (*j)->cro_type;
        val_field->number = "1";
        val_field->flexible = true;
        val_field->table = tab;
        file.file_path = db->data_folder+ "/" + (*i)->number + "_" + (*j)->number + ".txt";
        file.table = tab;
        delete (*j);
        j--;
        (*i)->fields.erase(j+1);
        tab->fields.push_back(id_field);
        tab->fields.push_back(val_field);
        new_tabs.push_back(tab);
        data_files.push_back( file );
        continue;
      }
  }

  // Переносим все созданные таблицы.
  for(vector<CTable *>::iterator i = new_tabs.begin(); i != new_tabs.end(); i++)
  {
    (*i)->sql_name = db->modify_tab_identificator((*i)->sql_name);
    if((*i)->sql_name.empty())
      warning_msgs.push_back("Неудалось подобрать уникальный идентификатор для таблицы множественного поля \"" + (*i)->cro_name + "\".");
    else
      db->tables.push_back(*i);
  }

  for( vector<CReference *>::iterator r = db->references.begin(); r != db->references.end(); r++ )
  {
    file.table = NULL;
    file.reference = *r;
    file.file_path = db->data_folder + "/" +  (*r)->cro_name + ".txt";
    data_files.push_back( file );
  }

  // Проверяем все ли файлы наместе:
  for(vector<SDataFile>::iterator df = data_files.begin(); df != data_files.end(); df++ )
    if( !(file_mode(df->file_path) & S_IFREG) )
    {
      warning_msgs.push_back("Файл данных \""+ df->file_path +"\" не найден.");
      df=data_files.erase(df);
	  df--;
    }
    else
    {
      if( file_size(df->file_path) + data_size  > numeric_limits<double>::max() )
        throw range_error("Database is very big.");
      data_size += file_size(df->file_path);
    }
  this->db = db;
  fr = NULL;
  current_file = data_files.end();
  _parse_status = BEGIN;
}

CDataParser::~CDataParser()
{
  if( fr != NULL )
  {
    delete fr;
    fr = NULL;
  }
}

int CDataParser::use_file(std::vector<SDataFile>::iterator data_file)
{
  if( fr != NULL )
  {
    delete fr;
    fr = NULL;
  }
  fr = new CFileReader();
  if( fr->bad() )
  {
    error_msg = "Неудалось инстанцировать разборщик.";
    return EXIT_FAILURE;
  }
  _parse_status = BEGIN;
  fr->open( (data_file->file_path).c_str() );
  if(fr->bad())
  {
    error_msg = "Неудалось открыть файл \"" + data_file->file_path + "\".";
    return EXIT_FAILURE;
  }
  current_file = data_file;
  return EXIT_SUCCESS;
}

string CDataParser::parse_next_line()
{
  _warning_type = NO_WARNINGS;
  if(fr->bad() || !fr->is_open() )
  {
    error_msg = "Файл не был открыт.";
    return "";
  }
  if(_parse_status == END )
    return "";

  string line = fr->get_until("\n");
  size_t line_size = line.length();
  string result;
  if( line.empty() )
  {
    if( fr->eof())
    {
      _parse_status = END;
      return "";
    }
    warning_msgs.push_back("Встречена пустая строка в файле \"" + current_file->file_path + "\".");
    _warning_type = EMPTY_LINE;
    return "";
  }

  // таблица (структура как у таблицы)
  if(current_file->table != NULL && current_file->reference == NULL)
  {
    // первая строчка - проверяем заголовок
    if(_parse_status == BEGIN)
    {
      size_t ibegin = 0;
      for(size_t iend = line.find(db->separator); iend != string::npos; ibegin = iend + 1, iend = line.find(db->separator, iend+1))
      {
        string tmp = line.substr(ibegin, iend - ibegin);
        // Находим нужное поле
        bool exist = false;
        for(vector<CField *>::iterator i = current_file->table->fields.begin(); i != current_file->table->fields.end(); i++)
          if( (*i)->cro_name == tmp )
          {
            current_file->fields_sequence.push_back((*i));
            exist = true;
            break;
          }
        if( !exist )
        {
          warning_msgs.push_back("Заголовок файла содержит строку, которая не является названием какого-либо поля.Возможно: 1. Структура банка в описании не совпадает со структурой данных. 2. Название поля содержит символ-разделитель. 3. База содержит ссылочное поле, не имеющее множественный атрибут.");
          current_file->fields_sequence.push_back(NULL);
        }
      }
      // Находим нужное поле
      string tmp = line.substr(ibegin, line.length() - ibegin);
      bool exist = false;
      for(vector<CField *>::iterator i = current_file->table->fields.begin(); i != current_file->table->fields.end(); i++)
        if( (*i)->cro_name == tmp)
        {
          current_file->fields_sequence.push_back((*i));
          exist = true;
          break;
        }
      if( !exist )
      {
        warning_msgs.push_back("Заголовок файла содержит строку, которая не является названием какого-либо поля.Возможно: 1. Структура банка в описании не совпадает со структурой данных. 2. Название поля содержит символ-разделитель. 3. База содержит ссылочное поле, не имеющее множественный атрибут.");
        current_file->fields_sequence.push_back(NULL);
      }

      // Если все значения, кроме "Системный номер" в fields_sequence NULL, то структура файла данных не соответствует структуре БД
      bool all_null = true;
      string finfo;
      for( vector<CField *>::iterator i = current_file->fields_sequence.begin(); i != current_file->fields_sequence.end(); i++ )
        if( (*i) != NULL )
        {
          if( (*i)->number != "0" )
            all_null = false;
          finfo += (*i)->sql_name + ", ";
        }

      if( all_null )
      {
        error_msg = "Файл данных \""+current_file->file_path+"\" не соответствует структуре базы \""+current_file->table->cro_name+"\", либо название одного из полей базы содержит символ-разделитель \""+db->separator+"\".";
        _parse_status = END;
        _warning_type = WRONG_HEADER;
        return "";
      }
      _parse_status = PROCESS;
      return "INSERT INTO " + current_file->table->sql_name + "(" + finfo.substr(0, finfo.length() - 2) + ") VALUES";
    }

    size_t ibegin = 0;
    result.clear();
    vector<CField *>::iterator curr_field = current_file->fields_sequence.begin();
    for(size_t iend = line.find(db->separator); iend != string::npos; ibegin = iend + 1, iend = line.find(db->separator, iend+1))
    {
      if(curr_field == current_file->fields_sequence.end())
      {
        warning_msgs.push_back("Строка файла \""+current_file->file_path+"\", начинающаяся с \"" + line.substr(0, 10) + "\" не соответствует структуре файла, либо содержит символ-разделитель.");
        _warning_type = WRONG_STRING;
        return "";
      }
      if( *curr_field == NULL )
      {
        curr_field++;
        continue;
      }
      string tmp = line.substr(ibegin, iend - ibegin);
      string tmp2 = fetch_field_data(line.substr(ibegin, iend - ibegin), *curr_field);
      if(tmp2 != "")
        result += tmp2 + ", ";
      else
        result += "NULL, ";
      curr_field++;
    }
    if(curr_field == current_file->fields_sequence.end() || curr_field + 1 != current_file->fields_sequence.end())
    {
      warning_msgs.push_back("Строка файла \""+current_file->file_path+"\", начинающаяся с \"" + line.substr(0, 10) + "\" не соответствует структуре файла, либо содержит символ-разделитель.");
      _warning_type = WRONG_STRING;
      return "";
    }
    if( *curr_field != NULL )
    {
      string tmp = fetch_field_data(line.substr(ibegin, line.length() - ibegin), *curr_field);
      if(tmp != "")
        result += tmp + ", ";
      else
        result += "NULL, ";
    }
    else
      result += "NULL, ";
    return "(" + result.substr(0, result.length() - 2) + ")";
  }
 /*

   // многозначное поле (структура (SN|MULTIVALUE))
   if(current_file->field != NULL && current_file->table != NULL && current_file->reference == NULL)
   {
     // первая строчка - проверяем заголовок
     if(_parse_status == BEGIN)
     {
       if( line != "SN|MULTIVALUE" )
       {
         error_msg = "Структура файла данных \""+current_file->file_path+"\" множественного поля \""+current_file->field->cro_name+"\" имеет неверный формат.";
         _parse_status = END;
         return "";
       }
       _parse_status = PROCESS;
       return "INSERT INTO " + current_file->table->sql_name + "(id, value) VALUES\\";
     }
     size_t sep_ind = line.find(db->separator);
     if(sep_ind == string::npos || !is_digit(line.substr(0, sep_ind)) )
       warning_msgs.push_back("Неверная структура файла данных \""+current_file->file_path+"\" множественного поля \""+current_file->field->cro_name+"\", начиная с : \""+line.substr(0, 10)+"\"");
     string tmp = fetch_field_data(line.substr(sep_ind, line.length() - sep_ind), current_file->field);
     if( tmp == "" )
     {
       warning_msgs.push_back("Неверная структура файла данных \""+current_file->file_path+"\" множественного поля \""+current_file->field->cro_name+"\", начиная с : \""+line.substr(0, 10)+"\"");
       return "";
     }
     return "(" + line.substr(0, sep_ind) + ", " + line.substr(sep_ind + 1, line.length() - sep_ind) + ")";
   }*/


  // ссылка (структура (SN|LINKSN))
  if( current_file->table == NULL && current_file->reference != NULL)
  {
    // первая строчка - проверяем заголовок
    if(_parse_status == BEGIN)
    {
      if( line != "SN|LINKSN" )
      {
        error_msg = "Структура файла данных \""+current_file->file_path+"\" ссылочного поля имеет неверный формат.";
        _parse_status = END;
        _warning_type = WRONG_HEADER;
        return "";
      }
      _parse_status = PROCESS;
      return "INSERT INTO " + current_file->reference->sql_name + "(id_from, id_to) VALUES";
    }
    size_t sep_ind = line.find(db->separator);
    if( sep_ind == string::npos || !is_digit(line.substr(0, sep_ind)) || !is_digit(line.substr(sep_ind + 1, line.length() - sep_ind - 1)))
    {
      warning_msgs.push_back("Неверная структура файла данных \""+current_file->file_path+"\" ссылочного поля, начиная с : \""+line.substr(0, 10)+"\"");
      _warning_type = WRONG_STRING;
      return "";
    }
    return "(" + line.substr(0, sep_ind) + ", " + line.substr(sep_ind + 1, line.length() - sep_ind) + ")";
  }
  processed_size += line_size;
  return result;
}

string CDataParser::fetch_field_data(std::string str, CField * field)
{

  if( str.empty() )
  {
    if( field->number != "0" )
      return "NULL";
    else
    {
      warning_msgs.push_back("Поле \""+field->cro_name+"\", не может иметь пустое значение (оно рассматривается как системный номер).");
      _warning_type = WRONG_STRING;
      return "";
    }
  }

  string result;

  size_t str_len = str.length();
  string data_type = get_data_type(str);

  // Расширение типа
  if( !data_type.empty() && data_type != field->sql_type)
  {
    if( db->is_extendable(field->sql_type, data_type) )
    {
      if( field->number == "0" )
      {
        warning_msgs.push_back("Поле \""+field->cro_name+"\", имеющее тип \""+field->cro_type+"\" не может содержать данные файла экспорта, начинающиеся с: \""+ str.substr(0, 10) +"\".");
        _warning_type = WRONG_STRING;
        return "";
      }
      if( field->flexible )
      {
        warning_msgs.push_back("Ранее присвоенный SQL-тип данных \""+ field->sql_type +"\" поля \""+field->cro_name+"\" был расширен до \""+data_type+"\".");
        if( data_type == db->TYPE_VARCHAR && field->cro_length.empty() )
          field->cro_length = "1";
        field->sql_type = data_type;
      }
      else
      {
        warning_msgs.push_back("SQL-тип данных \""+ field->sql_type +"\", заданный для поля \""+field->cro_name+"\" не подходит данным, начинающимся с \""+str.substr(0, 10)+"\".");
        _warning_type = WRONG_TYPE;
        return "";
      }
    }
    if( !db->is_extendable(data_type, field->sql_type) )
    {
      warning_msgs.push_back("Поле \""+field->cro_name+"\", имеющее тип \""+field->cro_type+"\" не может содержать данные файла экспорта, начинающиеся с: \""+ str.substr(0, 10) +"\".");
      _warning_type = WRONG_STRING;
      return "";
    }
  }


  result = str;
  // Расширение длины
  size_t field_len = atoi(field->cro_length.c_str());
  if( errno == ERANGE )
    throw range_error("Поле \""+field->cro_name+"\" имеет слишком большую длину \"" + field->cro_length + "\".");
  if( str_len > field_len )
  {
    if( field->flexible)
    {
      if(str_len < db->MAX_VARCHAR && field->sql_type == db->TYPE_VARCHAR)
      {
        stringstream ss;
        ss << str_len;
        field->cro_length = ss.str();
        warning_msgs.push_back("Длина поля \""+field->cro_name+"\" была расширена до \"" + field->cro_length +"\"");
      }
    } // flexible
    else
    {
      result = result.substr(0, field_len);
      warning_msgs.push_back("Данные поля \""+field->cro_name+"\" шире заданной длины (" + field->cro_length + "). Усечение.");
    }
  }

  if(field->sql_type == db->TYPE_DATE)
      result ="DATE '" + str.substr(6, 4) + "-" + str.substr(3, 2) + "-" + str.substr(0, 2) + "'";

  if( field->sql_type != db->TYPE_NUMBER && field->sql_type != db->TYPE_DATE && field->sql_type != db->TYPE_LONG && field->sql_type != db->TYPE_FLOAT )
  {
    // Экранируем спецсимволы, заковычиваем
    for(size_t i=0; i < result.length(); i++)
    {
/*      unsigned char result_i = (unsigned char)result[i];

      if( result_i <= 27)
      {
        result.erase(i);
        continue;
      }*/

/*
      if( (28 <= result_i && result_i <= 31) || result_i == 12 || result_i == 15 || result_i == '\n')
        result.replace(i, 1, "\\n");*/

      if( db->ESCAPABLE.find(result[i])!=string::npos )
      {
        result.insert(i, (string)db->ESCAPE_SYMBOL);
        i++;
        continue;
      }
    }
    result = "\'" + result + "\'";
  }

  return result;
}

string CDataParser::get_data_type(std::string str)
{
  size_t str_len = str.length();
  if(str_len == 0)
    return "";
  int numeric_type = 1;
  for(size_t i = 0; i < str_len; i++ )
  {
    if( !is_digit(str[i]) )
    {
      if(str[i] == '.' && numeric_type == 1)
      {
        numeric_type = 2;
        continue;
      }
      numeric_type = 0;
      break;
    }
  }

  if( numeric_type == 1 && str_len < 10)
  {
    long tmp = atol(str.c_str());
    if( errno == ERANGE )
      return db->TYPE_VARCHAR;
    if( tmp > numeric_limits<int>::max() )
      return db->TYPE_LONG;
    return db->TYPE_NUMBER;
  }

  if( numeric_type == 2 && str_len < 10)
  {
    (double)atof(str.c_str());
    if( errno == ERANGE )
      return db->TYPE_VARCHAR;
    return db->TYPE_FLOAT;
  }

  if( str_len == 5 && str[2] == ':' && is_digit(str[0]) && is_digit(str[1]) && is_digit(str[3]) && is_digit(str[4]) )
    return db->TYPE_TIME;
  if( str_len == 10 && str[2] == '.' && str[5] == '.' && is_digit(str[0]) && is_digit(str[1]) && is_digit(str[3]) && is_digit(str[4]) && is_digit(str[6]) && is_digit(str[7]) && is_digit(str[8]) && is_digit(str[9]))
    return db->TYPE_DATE;

  if( str.length() < db->MAX_VARCHAR )
    return db->TYPE_VARCHAR;

  return db->TYPE_CLOB;
}
