/*
1. Проверка всех возможных аргументов коммандной строки
   - проверка одновременного -m и -a
   - безаургументные -a, -m и -o
   - неверные пути
2. Файл списка
 НАЗВАНИЕ БАНКА
   - неожиданный конец файла в названии фанка
   - отсутствие названия банка
   - присутствие названия банка несколько раз
 ОПИСНАИЕ БАЗ
   - неверное количество разделителей в описании
     - больше
     - меньше
     - вообще нет
     - идут подряд
   - неожиданный конец файла в описании
   - мнемокод содержит символ |
   - мнемокод содержит цифры
   - номер банка не является числом
   - 100 баз с одинаковыми названиями и "плохими" мнемокодами
 ОПИСАНИЕ ХАРАКТЕРИСТИК
   - В мнемокоде базы есть |
   - Отсутствует описание характеристик какой-либо из баз
   - Порядок описания характеристик для баз банка отличается от порядка их следования в описании банка
   - Ссылочное поле немножественно
   - Ссылочное поле не имеет данных куда ссылается
   - У одной из баз есть ссылочное поле на другую, а у нее нет.
   - Внезапное окончание описания хар-к
   - Неверное число символов |
   - MAX_LEN+1 характеристик с одинаковыми названиями длиной MAX_LEN
*/

#include <iostream>
#include <stdexcept>
#include "dbstruct.h"

using namespace std;

class CTestDatabase : public CDatabase
{
public:
  bool test0();
  bool test1();
  bool test2();
  bool test3();
  bool test4();
  bool test(int);
  CTestDatabase(
    const size_t max_tab_identificator_len,   //!< Максимальная длина идентификатора таблицы
    const size_t max_field_identificator_len, //!< Максимальная длина идентификатора поля
    const size_t max_varchar_len,             //!< максимальная длина поля VARCHAR
    const char * escapable,                   //!< Строка, отражающая символы, которые необходимо экранировать (каждый символ строки)
    const char * escape_symbol,
    const char * integer_type,                //!< Тип, способный хранить число до 10 знаков
    const char * long_type,                   //!< Тип, способный хранить до C++ long символов
    const char * float_type,                  //!< Тип, способный хранить числа с плавающей точкой
    const char * clob_type,                   //!< Тип, способный хранить строки очень большой длины
    const char * varchar_type,                //!< Тип, способный хранить строки до max_varchar_len длиной
    const char * date_type,                   //!< Тип, способный хранить дату
    const char * time_type                    //!< Тип, способный храниить время
    ) : CDatabase(max_tab_identificator_len, max_field_identificator_len, max_varchar_len, escapable, escape_symbol, integer_type, long_type, float_type, clob_type, varchar_type, date_type, time_type)
    {};
};

bool CTestDatabase::test(int i)
{
  switch(i)
  {
  case 0: return test0();
  case 1: return test1();
  case 2: return test2();
  case 3: return test3();
  case 4: return test4();
  }
  return false;
}

bool CTestDatabase::test0()
{
  bool test = true;
  if(_translit_identifier("Лицо") != "lico") test = false;
  if(_translit_identifier("Персонал столовой") != "personal_stolovoi") test = false;
  if(_translit_identifier("1_Лицо") != "lico") test = false;
  if(_translit_identifier("_Персонал*?столовой'номер_1'") != "personal_stolovoi_nomer_1_") test = false;
  if(_translit_identifier("Этупеснюнезадушишьнеубъешьнаполянкулесавылезпьяныйеж") != "etupesnjunezadushishneubjeshnapoljankulesavilezpjaniiezh") test = false;
  if(_translit_identifier("_тупеснюнезадушишьнеубъешьнаполянкулесавылезпьяныйеж") != "tupesnjunezadushishneubjeshnapoljankulesavilezpjaniiezh") test = false;
  return test;
}

// Описание баз
bool CTestDatabase::test1()
{
  string str[] = {
    "1|Ли|цо|ЛЦ|1|0||2",
    "1|Лицо|ЛЦ|1|0|2",
    "1||||Лицо|Л|Ц|1|0|2",
    "Лицо",
    "1|Ли|цо|Л||1|0||2",
    "1|Лицо|5||1|0||2",
    "не_число|Лицо|ЛЦ|1|0||2"
  };
  for(size_t i=0; i < 7; i++)
  {
    CTable * tab = new CTable();
    if( !tab )
      throw runtime_error("Memory allocation error");
    int result;
    warning_msgs.clear();
    error_msg.clear();
    result = fetch_base_info(str[i], tab, false, 7);
    switch(i)
    {
    case 0: 
      if (result != EXIT_SUCCESS || tab->cro_name != "Ли|цо" || tab->short_name != "ЛЦ" || tab->number != "1" || tab->file != "1.txt") 
        return false;
      break;
    case 1:
      if (result != EXIT_FAILURE || error_msg != "Неверное описание мнемокода, либо имени базы." )
        return false;
      break;
    case 2:
      if (result != EXIT_FAILURE || error_msg != "Неверное описание мнемокода, либо имени базы." )
        return false;
      break;
    case 3:
      if (result != EXIT_FAILURE || error_msg != "Неверное описание базы, начинающееся с \"Лицо\"." )
        return false;
      break;
    case 4:
      if (result != EXIT_SUCCESS || tab->cro_name != "Ли|цо" || tab->short_name != "Л|" || tab->number != "1" || tab->file != "1.txt")
        return false;
      break;
    case 5:
      if (result != EXIT_SUCCESS || tab->cro_name != "Лицо" || tab->short_name != "5|" || tab->number != "1" || tab->file != "1.txt") 
        return false;
      break;
    case 6:
      if (result != EXIT_FAILURE || error_msg != "Неверный тип или отсутствуют данные в поле \"N\": \"не_число\"")
        return false;
      break;    
    }
    delete tab;
  }
  return true;
}

// Описание баз
bool CTestDatabase::test2()
{
  string str[] = {
    "1|Ли|цо|ЛЦ|1|0||2|Person",
    "1|Лицо|ЛЦ|1|0||2",
    "1||||Лицо|Л|Ц|1|0|2",
    "Лицо|ID",
    "1|Ли|цо|Л||1|0||2|123",
    "1|Лицо|5||1|0||2||",
    "не_число|Лицо|ЛЦ|1|0||2|Person"
  };
  for(size_t i=0; i < 7; i++)
  {
    CTable * tab = new CTable();
    if( !tab )
      throw runtime_error("Memory allocation error");
    int result;
    warning_msgs.clear();
    error_msg.clear();
    // N|Название базы|Имя|Тип|Кол-во наборов|Поле кода|Кол-во записей|SQL имя
    result = fetch_base_info(str[i], tab, true, 7);
    switch(i)
    {
    case 0: 
      if (result != EXIT_SUCCESS || tab->cro_name != "Ли|цо" || tab->short_name != "ЛЦ" || tab->number != "1" || tab->file != "1.txt" || tab->sql_name != "Person") 
        return false;
      break;
    case 1:
      if (result != EXIT_FAILURE || error_msg != "Неверное описание мнемокода, либо имени базы." )
        return false;
      break;
    case 2:
      if (result != EXIT_FAILURE || error_msg != "Неверное описание мнемокода, либо имени базы." )
        return false;
      break;
    case 3:
      if (result != EXIT_FAILURE || error_msg != "Неверное описание базы, начинающееся с \"Лицо|ID\"." )
        return false;
      break;
    case 4:
      if (result != EXIT_FAILURE || error_msg != "Неверный тип данных в поле \"SQL имя\": \"123\"")
        return false;
      break;
    case 5:
      if (result != EXIT_WARNING || warning_msgs.back() != "Мнемокод базы \"Лицо|5\" имеет формат, который может повлечь проблему разрешения на нее ссылок." || tab->cro_name != "Лицо|5" || tab->short_name != "|1" || tab->number != "1" || tab->file != "1.txt") 
        return false;
      break;
    case 6:
      if (result != EXIT_FAILURE || error_msg != "Неверный тип или отсутствуют данные в поле \"N\": \"не_число\"")
        return false;
      break;    
    }
    delete tab;
  }
  return true;
}

bool CTestDatabase::test3()
{
  string str[] = {
    "1|Фамилия|Т|10000||МН|",
    "1|Фамил|ия|Т|10000||МН|",
    "1|Фамил||||ия|Т|10000||МН|",
    "1|Фамил|ия|П|10000||МН|Л|2",
    "1|Фамил|ия|П|10000||МН|6|2",
    "1|Фамил|ия|П|10000||МН||62",
    "1|Фамил|ия|П|10000||МН|622"
  };
  for(size_t i = 0; i < 7; i++)
  {
    CField * field = new CField();
    CTable * tab = new CTable();
    field->table = tab;
    tab->_database = this;
    vector<CField *> list;
    if( !tab || !field)
      throw runtime_error("Memory allocation error");
    int result;
    warning_msgs.clear();
    error_msg.clear();
    size_t cursor = 0;
    result = fetch_field_info(&str[i], cursor, field, tab, &list, false);
    switch(i)
    {
    case 0: 
      if ( result != EXIT_SUCCESS || field->number != "1" || field->cro_name != "Фамилия" || field->cro_type != "Т" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != true || field->sql_name != "familija") 
        return false;
      break;
    case 1: 
      if ( result != EXIT_SUCCESS || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "Т" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != true || field->sql_name != "familija") 
        return false;
      break;
    case 2: 
      if ( result != EXIT_SUCCESS || field->number != "1" || field->cro_name != "Фамил||||ия" || field->cro_type != "Т" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != true || field->sql_name != "familija") 
        return false;
      break;
    case 3: 
      if ( result != 3 || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "П" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != true || field->sql_name != "familija" || field->link_to_base != "Л|" || field->link_to_field != "2") 
        return false;
      break;
    case 4: 
      if ( result != 3 || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "П" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != true || field->sql_name != "familija" || field->link_to_base != "6|" || field->link_to_field != "2") 
        return false;
      break;
    case 5: 
      if ( result != 3 || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "П" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != true || field->sql_name != "familija" || field->link_to_base != "|6" || field->link_to_field != "2") 
        return false;
      break;
    case 6: 
      if ( result != 3 || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "П" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != true || field->sql_name != "familija" || field->link_to_base != "62" || field->link_to_field != "2") 
        return false;
      break;
    }
    delete tab;
    delete field;
  }
  return true;
}

bool CTestDatabase::test4()
{
  string str[] = {
    "1|Фамилия|Т|10000||МН||SIRNAME|",
    "1|Фамил|ия|Т|10000||МН||SIRNAME|VARCHAR(50)",
    "1|Фамил||||ия|Т|10000||МН|||VARCHAR(50)",
    "1|Фамил|ия|П|10000||МН|Л|2|SIR_NAME|",
    "1|Фамил|ия|П|10000||МН|6|2||INTEGER",
    "1|Фамил|ия|П|10000||МН||62||TEXT",
    "1|Фамил|ия|П|10000||МН|622||"
  };
  for(size_t i = 0; i < 7; i++)
  {
    CField * field = new CField();
    CTable * tab = new CTable();
    field->table = tab;
    tab->_database = this;
    vector<CField *> list;
    if( !tab || !field)
      throw runtime_error("Memory allocation error");
    int result;
    warning_msgs.clear();
    error_msg.clear();
    size_t cursor = 0;
    result = fetch_field_info(&str[i], cursor, field, tab, &list, true);
    switch(i)
    {
    case 0: 
      if ( result != EXIT_SUCCESS || field->number != "1" || field->cro_name != "Фамилия" || field->cro_type != "Т" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != true || field->sql_type != "" || field->sql_name != "SIRNAME") 
        return false;
      break;
    case 1: 
      if ( result != EXIT_SUCCESS || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "Т" || field->attrib != "МН" || field->dict != "" || field->flexible != false || field->sql_type != "VARCHAR" || field->cro_length != "50" || field->sql_name != "SIRNAME") 
        return false;
      break;
    case 2: 
      if ( result != EXIT_SUCCESS || field->number != "1" || field->cro_name != "Фамил||||ия" || field->cro_type != "Т" || field->attrib != "МН" || field->dict != "" || field->flexible != false || field->sql_type != "VARCHAR" || field->cro_length != "50"|| field->sql_name != "familija") 
        return false;
      break;
    case 3: 
      if ( result != 3 || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "П" || field->link_to_base != "Л|" || field->link_to_field != "2" || field->flexible != true || field->sql_type != "" || field->sql_name != "SIR_NAME") 
        return false;
      break;
    case 4: 
      if ( result != 3 || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "П" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != false || field->sql_name != "familija" || field->link_to_base != "6|" || field->link_to_field != "2" || field->sql_type != "INTEGER" ) 
        return false;
      break;
    case 5: 
      if ( result != 3 || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "П" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != false || field->sql_name != "familija" || field->link_to_base != "|6" || field->link_to_field != "2" || field->sql_type != "TEXT" ) 
        return false;
      break;
    case 6: 
      if ( result != 3 || field->number != "1" || field->cro_name != "Фамил|ия" || field->cro_type != "П" || field->cro_length != "10000" || field->attrib != "МН" || field->dict != "" || field->flexible != true || field->sql_name != "familija" || field->link_to_base != "62" || field->link_to_field != "2" || field->sql_type != "" ) 
        return false;
      break;
    }
    delete tab;
    delete field;
  }
  return true;
}

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif // EXIT_SUCCESS
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif // EXIT_FAILURE
#ifndef EXIT_WARNING
#define EXIT_WARNING 2
#endif // EXIT_WARNING

int main()
{
  CTestDatabase TestDatabase(20, 30, 4000, "'", "'", "INTEGER\0", "LONG\0", "FLOAT\0", "TEXT\0", "VARCHAR\0", "DATE\0", "TIME\0");

  for(int i=0; i < 5; i++)
    if( TestDatabase.test(i) )
      cout << "OK : Test #" << i << endl;
    else
      cout << "ERR: Test #" << i << endl;
  return 0;
}
