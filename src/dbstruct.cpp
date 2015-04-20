/******************************************************************************
ProjectName: cro2sql
FileName: dbstruct.cpp 20090121
Subj: Реализуем классы, отражающие структуры БД ИСУБД CronosPlus и SQL-БД и их соответствия (сопряжение)
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


#include "dbstruct.h"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include "freader.h"
#include "misc.h"

using namespace std;

/****************************************************************************/
string CDatabase::get_warnings()
{
  string result;
  for(vector<string>::iterator wrn = warning_msgs.begin(); wrn != warning_msgs.end(); wrn++)
    result += *wrn + "\n";
  return result;
}

CTable::~CTable()
{
  while( !fields.empty() )
  {
    delete fields.back();
    fields.pop_back();
  }
}

CDatabase::~CDatabase()
{
  while( !tables.empty() )
  {
    delete tables.back();
    tables.pop_back();
  }
  while( !tables.empty() )
  {
    delete tables.back();
    tables.pop_back();
  }
}

CField * CField::operator=(CField * other)
{
  if(this == other)
    return this;
  this->cro_name = other->cro_name;
  this->link_to_base = other->link_to_base;
  this->link_to_field = other->link_to_field;
  this->sql_name = other->sql_name;
  this->number = other->number;
  this->cro_type = other->cro_type;
  this->sql_type = other->sql_type;
  this->cro_length = other->cro_length;
  this->dict = other->dict;
  this->attrib = other->attrib;
  this->flexible = other->flexible;
  this->table = other->table;
  return this;
}
/****************************************************************************/

CTable::CTable(CTable&)
{

}

CTable& CTable::operator=(CTable& other)
{
  return other;
}

CTable * CDatabase::get_table_by_sql_name(string tab_name)
{
  vector<CTable *>::iterator iter;
  for( vector<CTable *>::iterator iter = tables.begin(); iter != tables.end(); iter++ )
  {
    if( (*iter)->sql_name == tab_name )
      return (*iter);
  }
  return NULL;
}

CReference * CDatabase::get_ref_by_sql_name(string ref_name)
{
	  vector<CReference *>::iterator iter;
	  for( vector<CReference *>::iterator iter = references.begin(); iter != references.end(); iter++ )
	  {
	    if( (*iter)->sql_name == ref_name )
	      return (*iter);
	  }
	  return NULL;
}

CTable * CDatabase::get_table_by_cro_name(string tab_name)
{
  vector<CTable *>::iterator iter;
  for( vector<CTable *>::iterator iter = tables.begin(); iter != tables.end(); iter++ )
  {
    if( (*iter)->cro_name == tab_name )
      return (*iter);
  }
  return NULL;
}


CTable * CDatabase::get_table_by_short_name(string tab_name)
{
  vector<CTable *>::iterator iter;
  for( vector<CTable *>::iterator iter = tables.begin(); iter != tables.end(); iter++ )
  {
    if( (*iter)->short_name == tab_name )
      return (*iter);
  }
  return NULL;
}


CField * CTable::get_field_by_cro_name(string field_name)
{
  for(vector<CField *>::iterator i = fields.begin(); i != fields.end(); i++)
  {
    if( (*i)->cro_name == field_name )
      return (*i);
  }
  return NULL;
}

CField * CTable::get_field_by_sql_name(string field_name)
{
  for(vector<CField *>::iterator i = fields.begin(); i != fields.end(); i++)
  {
    if( (*i)->sql_name == field_name )
      return (*i);
  }
  return NULL;
}


CField * CTable::get_field_by_number(string number)
{
  for(vector<CField *>::iterator i = fields.begin(); i != fields.end(); i++)
  {
    if( (*i)->number == number )
      return (*i);
  }
  return NULL;
}

bool CDatabase::is_extendable(string type1, string type2)
{
  if(  (type1 == type2) ||
    ( type1 == TYPE_NUMBER && (type2 == TYPE_FLOAT || type2 == TYPE_LONG || type2 == TYPE_VARCHAR || type2 == TYPE_CLOB || type2 == TYPE_DATE || TYPE_TIME  ) ) ||
    ( type1 == TYPE_FLOAT && (type2 == TYPE_VARCHAR || type2 == TYPE_DATE ) ) ||
    ( type1 == TYPE_LONG && ( type2 == TYPE_VARCHAR || type2 == TYPE_CLOB || type2 == TYPE_DATE || TYPE_TIME  ) ) ||
    ( type1 == TYPE_VARCHAR && (type2 == TYPE_CLOB ) ) ||
    ( type1 == TYPE_DATE && (type2 == TYPE_CLOB || type2 == TYPE_VARCHAR) ) ||
    ( type1 == TYPE_TIME && (type2 == TYPE_CLOB || type2 == TYPE_VARCHAR) ) )
    return true;
  else
    return false;
}
/****************************************************************************/

string CDatabase::_translit_identifier(string str)
{
  string result;
  for(size_t i=0; i< str.length(); i++)
  {
    unsigned char symbol = (unsigned char)str[i];
    // пропускаем непичатные
    if( symbol <= 31 )
      continue;

    // Приводим к нижнему регистру символы
    if( (0xC0 <= symbol && symbol <= 0xDF) || (65 <= symbol && symbol <= 90) )
      symbol += 32;

    if( ('0' <=  symbol && symbol <= '9') || ('a' <= symbol && symbol <= 'z') )
      result += symbol;

    switch((char)symbol)
    {
    case 'а': result += "a"; break;
    case 'б': result += "b"; break;
    case 'в': result += "v"; break;
    case 'г': result += "g"; break;
    case 'д': result += "d"; break;
    case 'е': result += "e"; break;
    case 'ё': result += "e"; break;
    case 'ж': result += "zh"; break;
    case 'з': result += "z"; break;
    case 'и': result += "i"; break;
    case 'й': result += "i"; break;
    case 'к': result += "k"; break;
    case 'л': result += "l"; break;
    case 'м': result += "m"; break;
    case 'н': result += "n"; break;
    case 'о': result += "o"; break;
    case 'п': result += "p"; break;
    case 'р': result += "r"; break;
    case 'с': result += "s"; break;
    case 'т': result += "t"; break;
    case 'у': result += "u"; break;
    case 'ф': result += "f"; break;
    case 'х': result += "h"; break;
    case 'ц': result += "c"; break;
    case 'ч': result += "ch"; break;
    case 'ш': result += "sh"; break;
    case 'щ': result += "sh"; break;
    case 'ъ': result += "j"; break;
    case 'ы': result += "i"; break;
    case 'ь': break;
    case 'э': result += "e"; break;
    case 'ю': result += "ju"; break;
    case 'я': result += "ja"; break;
    }

    if( (32 <= symbol && symbol <= 47) || (58 <= symbol && symbol <= 64) || (91 <= symbol && symbol <= 96) || (128 <= symbol && symbol <= 191) )
      result += "_";
  }
  size_t tmp = result.find("__");
  while( tmp != string::npos )
  {
    result = result.erase(tmp, 1);
    tmp = result.find("__");
  }
  while( result.length() > 2 && ( result[0] > 'z' || result[0] < 'a' ) )
    result = result.erase(0, 1);
  if( result.length() == 1 && ( result[0] > 'z' || result[0] < 'a' ) )
    result = "";

  if( result.empty() )
    return "invalid_name";
  else
    return result;
}

bool add(string &str, size_t &pos, size_t max_len, string alphabet)
{
  if(alphabet.empty())
    return false;
  if( str[pos] == alphabet[alphabet.length() - 1])
  {
    if( pos == 0 )
    {
      if( str.length() == max_len )
        return false;
      str = alphabet[0] + str;
      pos++;
      return true;
    }
    if( !add(str, --pos, max_len, alphabet) )
      return false;
    str[pos] = alphabet[0];
    return true;
  }
  size_t tmp;
  if( str.empty() )
    if( max_len > 0 )
    {
      str += alphabet[0];
      return true;
    }
    tmp = alphabet.find(str[pos]);
    str[pos] = alphabet[tmp+1];
    return true;
}

string CDatabase::modify_tab_identificator(string tab_name)
{
  if( !is_lat_alpha(tab_name[0]) )
  {
    tab_name = "t" + tab_name;
  }
  if(tab_name.length() > MAX_TABIDENTLEN)
    tab_name = tab_name.substr(0, MAX_TABIDENTLEN);
  string result = tab_name;
  string addition = "0";
  size_t max_addition_len;
  if( tab_name.substr(0, 3) == "mv_" || tab_name.substr(0, 3) == "rf_")
    max_addition_len = MAX_TABIDENTLEN - 3;
  else
    max_addition_len = MAX_TABIDENTLEN - 1;
  while( get_table_by_sql_name(result) != NULL || get_ref_by_sql_name(result) != NULL)
  {
    size_t pos = addition.length() - 1;
    if( !add(addition, pos, max_addition_len, "0123456789abcdefghijklmnopqrstuvwxyz") )
      return "";
    if( tab_name.length() + addition.length() < MAX_TABIDENTLEN )
      result = tab_name + "_" + addition;
    if( tab_name.length() + addition.length() == MAX_TABIDENTLEN )
      result = tab_name + addition;
    if( tab_name.length() + addition.length() > MAX_TABIDENTLEN )
    {
      size_t tmp = tab_name.length() - (addition.length() - (MAX_TABIDENTLEN - tab_name.length()));
      result = tab_name.erase(tmp, tab_name.length() - tmp) + addition;
    }
  }
  return result;
}

string CTable::modify_field_identificator(string field_name)
{

  if( !is_lat_alpha(field_name[0]) )
  {
    field_name = "f" + field_name;
  }
  if(field_name.length() > _database->MAX_FIELDIDENTLEN)
    field_name = field_name.substr(0, _database->MAX_FIELDIDENTLEN);
  string result = field_name;
  string addition = "0";
  while( get_field_by_sql_name(result) != NULL )
  {
    size_t pos = addition.length() - 1;
    if( !add(addition, pos, _database->MAX_FIELDIDENTLEN - 1, "0123456789abcdefghijklmnopqrstuvwxyz") )
      return "";
    if( field_name.length() + addition.length() < _database->MAX_FIELDIDENTLEN )
      result = field_name + "_" + addition;
    if( field_name.length() + addition.length() == _database->MAX_FIELDIDENTLEN )
      result = field_name + addition;
    if( field_name.length() + addition.length() > _database->MAX_FIELDIDENTLEN )
    {
      size_t tmp = field_name.length() - (addition.length() - (_database->MAX_FIELDIDENTLEN - field_name.length()));
      result = field_name.erase(tmp, field_name.length() - tmp) + addition;
    }
  }
  return result;
}

string CDatabase::get_sql_type(CField * field)
{
  if(field->cro_type == field->TYPE_DATE)
    return TYPE_DATE;
  if(field->cro_type == field->TYPE_TIME)
    return TYPE_TIME;
  if(field->cro_type == field->TYPE_DICTIONARY )
    return TYPE_NUMBER;
  if(field->cro_type == field->TYPE_DIGITS)
  {
    if(atoi(field->cro_length.c_str()) <= 10)
      return TYPE_NUMBER;
    else
      return TYPE_LONG;
  }
  if(field->cro_type == field->TYPE_TEXT )
  {
    if((size_t)atoi(field->cro_length.c_str()) <= MAX_VARCHAR )
      return (string)TYPE_VARCHAR;
    else
      return TYPE_CLOB;
  }
  return "";
}
/****************************************************************************/
int CDatabase::fetch_base_info(const string str, CTable * tab, bool manual_mode, size_t delim_count)
{
  bool warn = false;
  if( manual_mode )
    delim_count++;
  // В строке символов "|" больше delim_count, то в названии есть символы "|", если меньше, то что-то не то с конфигом
  size_t delimiters[4] = {0, 0, 0, 0};
  size_t counter = 0;
  for(size_t j = str.length(); j > 0; j-- )
  {
    if( str[j] == '|' )
    {
      counter++;
      if( manual_mode && counter == 1 )
        delimiters[3] = j;
      if( counter == delim_count - 3  )
      {
        delimiters[2] = j;
        // Неважно какие 2 символа следуют за эти разделителем - они обязаны быть мнемокодом базы. Причем содержит ровно 2 символа.
        if( j - 3 <= 0 || str[j-3] != '|' )
        {
          error_msg = "Неверное описание мнемокода, либо имени базы.";
          return EXIT_FAILURE;
        }
        counter++;
        j -= 2;
        delimiters[1] = j - 1;
        continue;
      }
      delimiters[0] = j;
    }
  }
  if( counter < delim_count || delimiters[0] == 0 || delimiters[1] == 0 || (delimiters[1] - delimiters[0] < 1) || (delimiters[2] - delimiters[1] != 3) )
  {
    error_msg = "Неверное описание базы, начинающееся с \"" + str.substr(0, 10) + "\".";
    return EXIT_FAILURE;
  }
  tab->_database = this;
  tab->number = str.substr(0, delimiters[0]);
  if( tab->number.empty() || !is_digit(tab->number) )
  {
    error_msg = "Неверный тип или отсутствуют данные в поле \"N\": \"" + tab->number + "\"";
    return EXIT_FAILURE;
  }
  tab->file = tab->number + ".txt";
  tab->cro_name = str.substr(delimiters[0] + 1, delimiters[1] - delimiters[0] - 1);
  if( !is_printable(tab->cro_name) || tab->cro_name.empty() )
  {
    error_msg = "Неверный тип или отсутствуют данные в поле \"Название базы\": \"" + tab->cro_name + "\"";
    return EXIT_FAILURE;
  }
  if( manual_mode && (delimiters[3] + 1 < str.length()) )
  {
    tab->sql_name = str.substr(delimiters[3] + 1, str.length() - delimiters[3]);
    if( get_table_by_sql_name(tab->sql_name) != NULL )
    {
      error_msg = "Указанное вами \"SQL имя\": \"" + tab->sql_name + "\" не является уникальным в рамках данной БД. Попробуйте подобрать другое.";
      return EXIT_FAILURE;
    }
  }
  if( !manual_mode || tab->sql_name.empty())
  {
    tab->sql_name = _translit_identifier(tab->cro_name);
    tab->sql_name = modify_tab_identificator(tab->sql_name);
    if( tab->sql_name.empty() )
    {
      error_msg = "Неудалось подобрать уникальный идентификатор для \"" + tab->cro_name + "\".";
      return EXIT_FAILURE;
    }
  }
  if( !is_operand(tab->sql_name) )
  {
    error_msg = "Неверный тип данных в поле \"SQL имя\": \"" + tab->sql_name + "\"";
    return EXIT_FAILURE;
  }
  tab->short_name += str[delimiters[1] + 1];
  tab->short_name += str[delimiters[1] + 2];
  // Если мнемокод имеет вид |Ц, то могут возникать проблемы со ссылками на эту базу. Предупредим об этом пользователя и будем осторожны.
  if( (tab->short_name[0] == '|' && tab->short_name[1] == '|') || (tab->short_name[0] == '|' && is_digit(tab->short_name[1])) )
  {
    warning_msgs.push_back("Мнемокод базы \""+ tab->cro_name +"\" имеет формат, который может повлечь проблему разрешения на нее ссылок.");
    warn = true;
  }
  if( tab->short_name.empty() || !is_printable(tab->short_name) )
  {
    error_msg = "Неверный тип или отсутствуют данные в поле \"Имя\": \"" + tab->sql_name + "\"";
    return EXIT_FAILURE;
  }
  if(warn)
    return EXIT_WARNING;
  else
    return EXIT_SUCCESS;

}

int CDatabase::fetch_field_info(const string *str, size_t &cursor, CField * fld, CTable * base, vector<CField*> * links, bool manual_mode)
{
  size_t str_len = str->length();
  size_t delimiters[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  size_t delimiters_count = 6;
  if( manual_mode )
    delimiters_count = 8;
  size_t begin = cursor;
  // В мнемокоде базы могут содержаться символы || действуем так: выбераем первый |, идем до конца и с конца выделяем остальные
  for(;cursor < str_len && (*str)[cursor] != '\n'; cursor++)
  {
    if( delimiters[0] == 0 && (*str)[cursor] == '|' )
      delimiters[0] = cursor;
  }
  size_t counter = 0;
  for(size_t k = cursor; k > delimiters[0] && counter < delimiters_count - 1; k--)
  {
    if( (*str)[k] == '|' )
    {
      counter++;
      if( manual_mode && counter == 1 )
        delimiters[7] = k;
      if( (manual_mode && counter == 2) || (!manual_mode && counter == 1 && delimiters[5] == 0) )
      {
        if( manual_mode )
          delimiters[6] = k;
        if( !manual_mode )
          while( k < str->length() && (*str)[k] != '\n' )
            k++;

        if( k-4 < delimiters[0] )
        {
          error_msg = "Неверное описание одного из полей базы \""+base->cro_name+"\".";
          return EXIT_FAILURE;
        }
        // Если в ссылочном поле число букв меньше 2, тогда возможно несколько вариантов:
        // 1. в мнемокоде содержатся цифры
        // 2. в мнемокоде содержаться символы |
        // 3. мнемокод состоит из символа | и цифры.
        // Итого, возможно 8 вариантов: БЦ, Б|, ЦЦ, Ц|, |Б, |Ц, ||
        // Стоит заметить, что затем идет поле статус, значения которого фиксированы. Не содержат цифр.
        // ТО: Если идти с конца, пока не встретим букву, то это ситуации БЦ или |Б, если же первей попался символ |, то это ситуаци Б|, ||, |Ц, ЦЦ, Ц|, причем последняя легко отлавливается, а остальные неотличимы....
        // Исходя из вышесказанного считаем, что мнемокоды вида  Б|, ||, |Ц - проблемные, проверяем их и пишем варнинг пользователю, что могут возникнуть ошибки, остальные ситуации - ок
        if( (*str)[k-1] == '|' )
        {
          k--;
          delimiters[5] = k;
          if( manual_mode )
            counter++;
          continue;
        }
        if( !is_digit((*str)[k-1]) )
        {
          error_msg = "Неверное описание одного из ссылочных полей базы \""+base->cro_name+"\".";
          return EXIT_FAILURE;
        }
        k -= 2;
        size_t link_begin = k + 1;
        size_t alpha_counter = 0;
        while( k > delimiters[0] && alpha_counter < 2 && (*str)[k] != '|' )
        {
          if( !is_digit((*str)[k]) )
          {
            if((*str)[k-1] == ';' && k-2  > delimiters[0] )
            {
              alpha_counter = 0;
              k -= 2;
              continue;
            }
            alpha_counter++;
          }
          k--;
        }
        if(alpha_counter < 2)
        {
          // Частный случай |Ц и |Б
          //Ситуации |Ц и ЦЦ можно отличить только если - длина ссылочного поля - 1
          if( link_begin - k <= 3 && k-1 > delimiters[0] && (*str)[k-1] == '|')
            k--;
          //  Ситуация Б| или Ц|
          if( k - 2 > delimiters[0] && (*str)[k-2] == '|' )
            k -= 2;
        }
        delimiters[5] = k;
        if( manual_mode )
          counter++;
        continue;
      }
      delimiters[delimiters_count - counter] = k;
    }
  }
  counter++; // delimiter[0] уже был заполнен

  if( !manual_mode )
    delimiters[6] = cursor;

  if( counter < delimiters_count || delimiters[0] == 0 || delimiters[1] == 0 || delimiters[2] == 0 || delimiters[3] == 0 || delimiters[4] == 0 || delimiters[5] == 0 || (delimiters[1] - delimiters[0] < 1) || (delimiters[2] - delimiters[1] < 1) || begin - delimiters[0] < 1 )
  {
    error_msg = "Неверное описание поля, начиная с \"" + str->substr(0, 10) + "\".";
    return EXIT_FAILURE;
  }
  fld->table = base;
  fld->number = str->substr(begin, delimiters[0] - begin);
  if( fld->number.empty() || !is_digit(fld->number))
  {
    error_msg = "Неверный тип или отсутствуют данные в описании \"Номер\" базы \"" + base->cro_name + "\": \"" + str->substr(0, 10) + "\"";
    return EXIT_FAILURE;
  }
  fld->cro_name = str->substr(delimiters[0] + 1, delimiters[1] - delimiters[0] - 1);
  if( fld->cro_name.empty() || !is_printable(fld->cro_name) )
  {
    error_msg = "Неверный тип или отсутствуют данные в описании \"Название поля\" базы \"" + base->cro_name + "\": \"" + str->substr(delimiters[0], 10) + "\"";
    return EXIT_FAILURE;
  }
  fld->cro_type = str->substr(delimiters[1] + 1, delimiters[2] - delimiters[1] - 1);
  // TODO: TYPE_SECURITY_LEVEL - это поле уровень доступа. Я его рассматриваю
  //       как нормальное поле. Надо бы с ним поаккуратней...
  if( fld->cro_type.empty() ||
    (fld->cro_type != fld->TYPE_DIGITS &&
    fld->cro_type != fld->TYPE_TEXT &&
    fld->cro_type != fld->TYPE_DICTIONARY &&
    fld->cro_type != fld->TYPE_DATE &&
    fld->cro_type != fld->TYPE_TIME &&
    fld->cro_type != fld->TYPE_FILE &&
    fld->cro_type != fld->TYPE_TEMP_FILE &&
    fld->cro_type != fld->TYPE_LINK_DIRECT &&
    fld->cro_type != fld->TYPE_LINK_INVERSE &&
    fld->cro_type != fld->TYPE_LINK_MULTY &&
    fld->cro_type != fld->TYPE_SECURITY_LEVEL) )
  {
    error_msg = "Неверный тип или отсутствуют данные в описании \"Тип\" поля \"" + fld->cro_name + "\" базы \"" + base->cro_name + "\".";
    return EXIT_FAILURE;
  }
  fld->cro_length = str->substr(delimiters[2] + 1, delimiters[3] - delimiters[2] - 1);
  if( !is_digit(fld->cro_length) )
  {
    error_msg = "Неверный тип или отсутствуют данные в описании \"Длина поля\" поля \"" + fld->cro_name + "\" базы \"" + base->cro_name + "\".";
    return EXIT_FAILURE;
  }
  fld->dict = str->substr(delimiters[3] + 1, delimiters[4] - delimiters[3] - 1);
  if( !is_printable(fld->dict) )
  {
    error_msg = "Неверный тип или отсутствуют данные в описании \"Словарь\" поля \"" + fld->cro_name + "\" базы \"" + base->cro_name + "\".";
    return EXIT_FAILURE;
  }
  fld->attrib = str->substr(delimiters[4] + 1, delimiters[5] - delimiters[4] - 1);
  fld->flexible = true;
  if( manual_mode )
  {
    fld->sql_name = str->substr(delimiters[6] + 1, delimiters[7] - delimiters[6] - 1);
    if( base->get_field_by_sql_name(fld->sql_name) != NULL )
    {
      error_msg = "Указанное вами \"SQL имя\": \"" + fld->sql_name + "\" не является уникальным в рамках таблицы. \"" + base->sql_name + "\". Попробуйте подобрать другое.";
      return EXIT_FAILURE;
    }
    string tmp = str->substr(delimiters[7] + 1, cursor - delimiters[7] - 1 );
    size_t tl = tmp.find("(");
    size_t tr = tmp.find(")", tl + 1);
    if( tl != string::npos && tr != string::npos && is_digit(tmp.substr(tl + 1, tr - tl - 1)) )
    {
      fld->cro_length = tmp.substr(tl + 1, tr - tl - 1);
      fld->sql_type = tmp.erase(tl, tr - tl + 1);
    }
    else
      fld->sql_type = tmp;
    if(!fld->sql_type.empty())
      fld->flexible = false;
  }
  if( !manual_mode || fld->sql_name.empty() )
  {
    fld->sql_name = _translit_identifier(fld->cro_name);
    fld->sql_name = base->modify_field_identificator(fld->sql_name);
    if( fld->sql_name.empty() )
    {
      error_msg = "Неудалось подобрать уникальный идентификатор для \"" + fld->cro_name + "\".";
      return EXIT_FAILURE;
    }
  }
  if( !is_operand(fld->sql_name) )
  {
    error_msg = "Неверный тип или отсутствуют данные в описании \"SQL имя\" поля \"" + fld->cro_name + "\" базы \"" + base->cro_name + "\".";;
    return EXIT_FAILURE;
  }

  if( fld->cro_type == fld->TYPE_LINK_DIRECT || fld->cro_type == fld->TYPE_LINK_INVERSE || fld->cro_type == fld->TYPE_LINK_MULTY )
  {
    string slinks = str->substr(delimiters[5] + 1, delimiters[6] - delimiters[5] - 1);
    // выделяем все ссылки
    size_t ibegin = 0, iend = slinks.find(";");
    while( iend != string::npos)
    {
      string link = slinks.substr(ibegin, iend - ibegin);
      // Если нет описания ссылочного поля, то возможна потеря данных о связях.
      if( link.empty() || link.length() < 3)
      {
        warning_msgs.push_back("Поле \"" + fld->cro_name + "\" имеет ссылочный тип, но описание ссылки ошибочно.\n");
        return EXIT_WARNING;
      }
      CField * fld2 = new CField();
      fld2->operator=(fld);
      fld->link_to_base.clear();
      fld2->link_to_base += link[0];
      fld2->link_to_base += link[1];
      fld2->link_to_field = link.substr(2, link.length() - 2);
      links->push_back(fld2);
      ibegin = iend + 1;
      iend = slinks.find(";", ibegin);
    }
    // Если нет описания ссылочного поля, то возможна потеря данных о связях.
    if( slinks.substr(ibegin, slinks.length() - ibegin).empty() || slinks.substr(ibegin, slinks.length() - ibegin).length() < 3)
    {
      warning_msgs.push_back("Поле \"" + fld->cro_name + "\" имеет ссылочный тип, но отсутствует информация о ссылке.\n");
      return EXIT_WARNING;
    }
    fld->link_to_base.clear();
    fld->link_to_base += slinks[ibegin];
    fld->link_to_base +=  slinks[ibegin + 1];
    fld->link_to_field = slinks.substr(ibegin + 2, slinks.length() - ibegin - 2);
    links->push_back( fld );
    return 3;
  }
  return EXIT_SUCCESS;
}

int CDatabase::links_resolve(vector<CField *> * links)
{
  bool warn = false;
  vector<CField *>::iterator i = links->begin();
  while( i != links->end() )
  {
    CField * from_field = *i;
    CTable * to_table = from_field->table->_database->get_table_by_short_name(from_field->link_to_base);
    if( to_table == NULL )
    {
      warning_msgs.push_back("Таблица на которую указывает ссылка \""+from_field->cro_name+"\" не найдена. Возможна потеря данных по связям.");
      warn = true;
      delete (*i);
      i=links->erase(i);
      continue;
    }
    CField * to_field = NULL;
    vector<CField *>::iterator j;
    for(j = links->begin(); j != links->end(); j++)
    {
      if( from_field->link_to_field == (*j)->number && (*j)->table->short_name == from_field->link_to_base)
      {
        to_field = *j;
        break;
      }
    }
    if( to_field == NULL )
    {
      warning_msgs.push_back( "Поле, на которое указывает ссылка \""+from_field->cro_name+"\" не найдено. Возможна потеря данных по связям.");
      warn = true;
      delete (*i);
      i=links->erase(i);
      continue;
    }
    if( from_field->number == to_field->number && from_field->table->cro_name == to_field->table->cro_name )
    {
      warning_msgs.push_back( "Ссылка \""+from_field->cro_name+"\" является кольцевой. Возможна потеря данных по связям.");
      warn = true;
    }
    if( (*i)->attrib.find("МН") == string::npos )
    {
      warning_msgs.push_back("Ссылка \""+(*i)->cro_name+"\" не имеет множественного атрибута. Возможна потеря данных по связям.");
      warn = true;
    }
    if( (*i)->cro_type == (*i)->TYPE_LINK_INVERSE )
	{
	  i++;
      continue; // Не интересует, если на них не указывает какой-либо LINK_TYPE_DIRECT
	}
    if( (*i)->cro_type == (*i)->TYPE_LINK_DIRECT )
      if( to_field->cro_type != (*i)->TYPE_LINK_INVERSE )
      {
        warning_msgs.push_back("Поле на которое указывает ссылка \""+from_field->cro_name+"\" должно иметь иной тип ссылки. Возможна потеря данных по связям.");
        warn = true;
        delete (*i);
        i=links->erase(i);
        continue;
      }
      if( (*i)->cro_type == (*i)->TYPE_LINK_MULTY )
        if( to_field->cro_type != (*i)->TYPE_LINK_MULTY )
        {
          warning_msgs.push_back("Поле на которое указывает ссылка \""+from_field->cro_name+"\" должно иметь иной тип ссылки. Возможна потеря данных по связям.");
          warn = true;
          delete (*i);
          i=links->erase(i);
          continue;
        }
        CReference * tmp_ref = new CReference();
        if( !tmp_ref )
          throw runtime_error("Невозможно выделить память под ссылку");
        tmp_ref->cro_name = from_field->table->number + "_" + from_field->number + "_" + to_table->number;
        tmp_ref->from = from_field->table;
        tmp_ref->to = to_table;
        tmp_ref->sql_name = modify_tab_identificator("ln_" + from_field->table->sql_name + "_" + from_field->sql_name + "_" + to_table->sql_name);
        if( tmp_ref->sql_name.empty() )
        {
          warning_msgs.push_back("Неудалось подобрать уникальное имя для ссылки.");
          return EXIT_WARNING;
        }
        references.push_back(tmp_ref);
		
		// Немного неловко сделано, потому что приходится обратные ссылки
		// несколько раз обходить, но зато нет утечек по памяти...
		if( i != j)
		{
          CField* rmi = *i;
          CField* rmj = *j;
          links->erase(remove(links->begin(), links->end(), rmj), links->end());
          links->erase(remove(links->begin(), links->end(), rmi), links->end());
          delete(rmi);
          delete(rmj);
		  i = links->begin();
        }
		else
		{
			delete(*i);
			i=links->erase(i);
		}
  }
  if(warn)
    return EXIT_WARNING;
  else
    return EXIT_SUCCESS;
}

int CDatabase::read_from_file(string path, string name)
{
  CFileReader * dbstruct_file = new CFileReader( path );
  if( dbstruct_file->bad() )
  {
    error_msg = "Невозможно открыть файл.";
    return EXIT_FAILURE;
  }
  if( name.empty() )
    warning_msgs.push_back("Имя банка данных не было задано. Используем первый банк из файла.");
  else
    if( !dbstruct_file->skip_until("Наименование банка:  " + name) )
    {
      error_msg = "Описание банка в файле не найдено.";
      return EXIT_FAILURE;
    }

  if( !dbstruct_file->skip_until("Список баз:") )
  {
    error_msg = "Файл не содержит списка баз.";
    return EXIT_FAILURE;
  }
  dbstruct_file->ignore(1);
  string head = dbstruct_file->get_until("\n");
  size_t ind[3];
  ind[0] = strlen("N|Название базы|Имя|Тип|Кол-во наборов|Поле кода");
  ind[1] = ind[0] + strlen("|Кол-во записей");
  ind[2] = ind[1] + strlen("|SQL имя");
  if( head.substr(0, ind[0]) != "N|Название базы|Имя|Тип|Кол-во наборов|Поле кода" )  {
    error_msg = "Структура списка баз неверна.";
    return 0;
  }
  size_t delim_count = 6;
  if( head.substr(ind[0], ind[1] - ind[0]) == "|Кол-во записей" )
    delim_count++;
  else
    ind[1] = ind[0];

  bool manual_base_names = false;
  // Проверяем заданы ли sql имена вручную
  if( head.substr(ind[1], ind[2] - ind[1]) == "|SQL имя" )
    manual_base_names = true;

  for(string line = dbstruct_file->get_until("\n"); !line.empty() ;line = dbstruct_file->get_until("\n"))
  {
    CTable * tmp_tab = new CTable();
    if( !tmp_tab )
      throw runtime_error("Не удалось выделить память под таблицу.");
    if( fetch_base_info(line, tmp_tab, manual_base_names, delim_count) == EXIT_FAILURE)
      return EXIT_FAILURE;
    tables.push_back(tmp_tab);
  }

  /**разбор полей************/
  // Все ссылочные поля будем списывать в вектор ссылочных полей
  vector<CField *> links;
  // считываем кусок файла, содержащий информацию о банках в память
  string db_struct = dbstruct_file->get_until("==========================\n##############################################################");
  size_t db_struct_len = db_struct.length();
  if( tables.size() != 0 && db_struct_len == 0 )
  {
    error_msg = "Отсутствуют описания баз.";
    return EXIT_FAILURE;
  }
  for(vector<CTable *>::iterator i = tables.begin(); i != tables.end(); i++ )
  {
    bool manual_field_names = false;
    size_t cursor = 0;
    cursor = db_struct.find("База  :" + (*i)->cro_name);
    if(cursor == string::npos)
    {
      error_msg = "Отсутствует описание базы \"" + (*i)->cro_name + "\".";
      return EXIT_FAILURE;
    }
    size_t first = cursor;
    cursor += ("База  :" + (*i)->cro_name).length() + 1;
    if( cursor + strlen("Состав полей базы\nN поля| Название поля|Тип|Длина поля|Словарь|Статус|Связь с базами") + 8 > db_struct_len )
    {
      error_msg = "Неверное описание базы \"" + (*i)->cro_name + "\".";
      return EXIT_FAILURE;
    }
    cursor += strlen("Состав полей базы\nN поля| Название поля|Тип|Длина поля|Словарь|Статус|Связь с базами") + 1;
    if( db_struct.substr(cursor, ((string)"SQL имя|SQL тип").length()) == "SQL имя|SQL тип" )
    {
      manual_field_names = true;
      cursor += ((string)"SQL имя|SQL тип").length() + 1;
    }
    if( cursor + 8 > db_struct_len )
    {
      error_msg = "Неверное описание базы \"" + (*i)->cro_name + "\".";
      return EXIT_FAILURE;
    }
    for(; cursor < db_struct_len && db_struct[cursor] != '\n'; cursor++)
    {
      CField * tmp_field = new CField();
      if( !tmp_field )
        throw runtime_error("Невозможно выделить память под поле");
      int result = fetch_field_info(&db_struct, cursor, tmp_field, *i, &links, manual_field_names );
      if( result == 3 || (result == 2 && (tmp_field->cro_type == tmp_field->TYPE_LINK_DIRECT || tmp_field->cro_type == tmp_field->TYPE_LINK_INVERSE || tmp_field->cro_type == tmp_field->TYPE_LINK_MULTY ) ))
        continue;
      if( result == EXIT_FAILURE )
        return EXIT_FAILURE;
      if( tmp_field->sql_type.empty() )
        tmp_field->sql_type = get_sql_type( tmp_field );
      if( tmp_field->sql_type.empty() )
      {
        warning_msgs.push_back("Поле \"" + tmp_field->cro_name + "\" имеет неконвертируемый тип данных: \"" + tmp_field->cro_type + "\"");
      }
      else
        (*i)->fields.push_back(tmp_field);
    }
    if( (*i)->fields.empty() )
      warning_msgs.push_back("В файле описаний не найдена информация о полях базы \"" + (*i)->cro_name + "\".");
    db_struct = db_struct.replace(first, cursor - first, "");
    db_struct_len = db_struct.length();
  }
  if( tables.empty() )
  {
    error_msg = "В файле описаний не найдена информация о базах.";
    return EXIT_FAILURE;
  }
  delete dbstruct_file;

  ///Когда разобраны все поля и базы, есть возможность проверить правильность ссылок и, в случае верной ссылки, создать соответствующий объект CReference
  links_resolve(&links);
  if( !links.empty() )
  {
    for(vector<CField *>::iterator i = links.begin(); i != links.end(); i++)
    {
      delete (*i);
      *i = NULL;
    }
  }

  if( warning_msgs.empty() )
    return EXIT_SUCCESS;
  else
    return EXIT_WARNING;
}

/****Генератор**********************************************************/

string CDatabase::generate_DDL()
{
  string result;
  for(vector<CTable *>::iterator i = tables.begin(); i != tables.end(); i++)
  {
    result += "CREATE TABLE " + (*i)->sql_name + "(\n";
    for(vector<CField *>::iterator j = (*i)->fields.begin(); j != (*i)->fields.end(); j++)
    {
      if( (*j)->number == "0" && (*j)->flexible)
      {
        result += (*j)->sql_name + " " + (*j)->sql_type + " PRIMARY KEY,\n";
        continue;
      }
      if( (*j)->sql_type == TYPE_VARCHAR )
        result += (*j)->sql_name + " " + (*j)->sql_type + "(" + (*j)->cro_length+ ")" + ",\n";
      else
        result += (*j)->sql_name + " " + (*j)->sql_type + ",\n";
    }
    result = result.substr(0, result.length() - 2) + "\n";
    result += ");\n\n";
  }
  for(vector<CReference *>::iterator i = references.begin(); i != references.end(); i++)
  {
    // Уточняем типы данных системных номеров
    vector<CField *>::iterator from_i = (*i)->from->fields.begin();
    for(; from_i != (*i)->from->fields.end(); from_i++ )
      if((*from_i)->number == "0")
        break;
    vector<CField *>::iterator to_i = (*i)->to->fields.begin();
    for(; to_i != (*i)->to->fields.end(); to_i++ )
      if((*to_i)->number == "0")
        break;
    if( from_i == (*i)->from->fields.end() || to_i == (*i)->to->fields.end() )
      warning_msgs.push_back("Неудалось найти системный номер одной из ссылочных таблиц ссылки: \"" + (*i)->cro_name + "\".");
    result += "CREATE TABLE " + (*i)->sql_name + "(\nid_from " + (*from_i)->sql_type + ",\nid_to " + (*to_i)->sql_type + "\n);\n\n";
  }
  return result;
}
