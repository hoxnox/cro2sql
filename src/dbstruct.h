/******************************************************************************
ProjectName: cro2sql
FileName: dbstruct.h 20081225
Subj: Определяет классы, отражающие структуры БД ИСУБД CronosPlus и SQL-БД и их соответствия (сопряжение)
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

#ifndef __DATABASE_STRUCT__
#define __DATABASE_STRUCT__ 1

#include <string>
#include <vector>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif // EXIT_SUCCESS
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif // EXIT_FAILURE
#ifndef EXIT_WARNING
#define EXIT_WARNING 2
#endif // EXIT_WARNING


class CTable;

/**
* \class Field
* \brief Класс, отражающий структуру поля реляционной БД (характеристики ИСУБД CronosPlus).
**/
class CField
{
	public:
    const char * TYPE_DIGITS;
    const char * TYPE_TEXT;
    const char * TYPE_DICTIONARY;
    const char * TYPE_DATE;
    const char * TYPE_TIME;
    const char * TYPE_FILE;
    const char * TYPE_TEMP_FILE;
    const char * TYPE_LINK_DIRECT;
    const char * TYPE_LINK_INVERSE;
    const char * TYPE_LINK_MULTY;
    const char * TYPE_SECURITY_LEVEL;
    CField() : TYPE_DIGITS("Ц\0"),
               TYPE_TEXT("Т\0"),
               TYPE_DICTIONARY("С\0"),
               TYPE_DATE("Д\0"),
               TYPE_TIME("В\0"),
               TYPE_FILE("Ф\0"),
               TYPE_TEMP_FILE("ВФ\0"),
               TYPE_LINK_DIRECT("П\0"),
               TYPE_LINK_INVERSE("О\0"),
               TYPE_LINK_MULTY("ПО\0"),
               TYPE_SECURITY_LEVEL("УД\0"){};
		std::string cro_name;       //!< Название
    std::string link_to_base;   //!< Если поле ссылочное, указывается направление ссылки (база)
    std::string link_to_field;  //!< Если поле ссылочное, указывается направление ссылки (поле)
    std::string sql_name;       //!< sql имя
    std::string number;         //!< Номер
		std::string cro_type;       //!< Тип данных cro
    std::string sql_type;       //!< Тип данных sql
		std::string cro_length;     //!< Длина (размерность) cro, 0 - если нет
    std::string dict;           //!< Имя словарной таблицы (базы)
		std::string attrib;         //!< Дополнительные атрибуты (в CronosPlus, например, статус характеристики)
    bool flexible;              //!< Флаг возможности внесения изменений в атрибуты поля
    CTable * table;             //!< Поле принадлежит таблице
    CField * operator=(CField * other);
};


class CDatabase;

/**
* \class Table
* \brief Класс, отражающий структуру таблицы БД (базы ИСУБД CronosPlus).
**/
class CTable
{
	public:
		std::string cro_name;           //!< Название таблицы (базы) cro.
    std::string sql_name;           //!< Название таблицы (базы) sql.
    std::string short_name;         //!< Мнемокод базы.
    std::string number;             //!< Номер базы.
    std::string file;               //!< Имя файла, содержащего экспорт данных из таблицы.
    CDatabase * _database;          //!< Таблица принадлежит БД
		std::vector<CField *> fields;   //!< Поля таблицы

    /// Ищет поле по его cro имени, возращает NULL, если не нашел
    CField * get_field_by_cro_name(std::string);
    /// Ищет поле по его sql имени, возращает NULL, если не нашел
    CField * get_field_by_sql_name(std::string);
    /// Ищет поле по его номеру, возращает NULL, если не нашел
    CField * get_field_by_number(std::string);
    /// Пытается отыскать уникальный идентификатор поля на основе существующего. Если это не получается, возвращает пустую строку.
    std::string modify_field_identificator(std::string);
    CTable() {};
    ~CTable();
private:
  CTable(CTable&);
  CTable& operator=(CTable& other);
};

/**
* \class Reference
* \brief Класс, отражающий отношение реляционной БД.
* \author Psycho
*
* ИСУБД CronosPlus имеет отличную от реляционной БД логику функционирования. Понятие типа связи используется в ИСУБД только для манипуляции объектами (например при удалении связных объектов).
* \copyright (c) 2008 psycho.ukcu@gmail.ru
**/
class CReference
{
	public:
    std::string sql_name;
	std::string cro_name;
    CTable * from;
    CTable * to;
};

/**
* \class CDatabase
* \brief Класс отражающий структуру банка данных (базы данных).
**/
class CDatabase
{
	public:
		std::string name;                         //!< Название БД
    std::string data_folder;                  //!< Путь к папке, в которой расположен экспорт данных БД
    const size_t MAX_TABIDENTLEN;             //!< Максимальная длина идентификатора таблицы
    const size_t MAX_FIELDIDENTLEN;           //!< максимальная длина идентификатора поля
    const size_t MAX_VARCHAR;                 //!< Максимальное количество символов для типа VARCHAR
    std::string separator;                    //!< Разделитель между данными в файлах экспорта
		std::vector<CTable *>  tables;            //!< Таблицы БД (базы ИСУБД)
		std::vector<CReference *> references;     //!< Отношения между таблицами БД (ссылки между базами ИСУБД)

    /// Ищет таблицу по ее cro имени, возвращет NULL, если не находит.
    CTable * get_table_by_cro_name(std::string);
    /// Ищет таблицу по ее sql имени, возвращет NULL, если не находит.
    CTable * get_table_by_sql_name(std::string);
    /// Ищет таблицу по мнемокоду, возвращет NULL, если не находит.
    CTable * get_table_by_short_name(std::string);
    /// Ищет ссылку по sql имени, возвращет NULL, если не находит.
    CReference * get_ref_by_sql_name(std::string);
    ///
    std::string _translit_identifier(std::string str);
    /// Изменить иентификатор таблицы. Если невозможно отыскать уникальное имя, возвращает пустую строку
    std::string modify_tab_identificator(std::string);
    /// Считать структуру БД из файла. Возвращает 0, если ошибок не было, 1 если были ошибки и 2, если были предупреждения
    int read_from_file(std::string path, std::string name);
    /// Возвратить последнюю ошибку
    std::string get_last_error() const {return error_msg;};
    /// Возвратить форматированную строку предуреждающих сообщений.
    std::string get_warnings();
    /// Возвратить по структуре БД SQL DDL выражения
    std::string generate_DDL();
    const std::string ESCAPABLE;
    const std::string ESCAPE_SYMBOL;
    const char * TYPE_NUMBER;
    const char * TYPE_LONG;
    const char * TYPE_FLOAT;
    const char * TYPE_VARCHAR;
    const char * TYPE_CLOB;
    const char * TYPE_DATE;
    const char * TYPE_TIME;
    CDatabase(
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
      ) :
      MAX_TABIDENTLEN(max_tab_identificator_len),
      MAX_FIELDIDENTLEN(max_field_identificator_len),
      MAX_VARCHAR(max_varchar_len),
      ESCAPABLE(escapable),
      ESCAPE_SYMBOL(escape_symbol),
      TYPE_NUMBER(integer_type),
      TYPE_LONG(long_type),
      TYPE_FLOAT(float_type),
      TYPE_VARCHAR(varchar_type),
      TYPE_CLOB(clob_type),
      TYPE_DATE(date_type),
      TYPE_TIME(time_type)
      {};
    /*
    // Настройка полумолчанию - MySQL
    CDatabase() : MAX_IDENTLEN(MAX_IDENTIFICATOR_LEN),
                  MAX_VARCHAR(2000),
                  ESCAPABLE("\\"),
                  TYPE_NUMBER("INTEGER\0"),
                  TYPE_LONG("LONG\0"),
                  TYPE_FLOAT("FLOAT\0"),
                  TYPE_CLOB("TEXT\0"),
                  TYPE_VARCHAR("VARCHAR\0"),
                  TYPE_DATE("DATE\0"),
                  TYPE_TIME("TIME\0")
                  {};*/

    ~CDatabase();
    /// Возвращает true, если SQL-тип аргумент 1 расширяем на 2 false в противном случае
    bool is_extendable(std::string , std::string);
protected:
  /// Выделить из строки данные о базе. Последний параметр - есть ли дополнительные SQL-ные поля
  int fetch_base_info(const std::string, CTable *, bool, size_t delim_count);
  /// Выделить из строки данные о поле. Последний параметр - есть ли дополнительные SQL-ные поля
  /**
  * \return EXIT_SUCCESS при правильном выделении обычного (нессылочного) поля. \
            EXIT_FAILURE при возникновении критической ошибки (для получения информации по ней используется get_last_error()), \
            EXIT_WARNING при возникновении ситуаций, о которых должен быть проинформирован пользователь (для получения по ним информации используется get_warnings()). \
            3 в случае, если правильно было выделено ссылочное поле
  **/
  int fetch_field_info(const std::string *, size_t &, CField *, CTable *, std::vector<CField *> *, bool);
  /// Разобрать ссылки. Для верных создать соответствующие объекты CReference
  int links_resolve(std::vector<CField *> * links);
  /// Определяет SQL тип, который может подходить для данного поля. По данным тип уточняется.
  std::string get_sql_type(CField *);
  // Конструктор копирования запрещен
  CDatabase(CDatabase&) :
   MAX_TABIDENTLEN(0),
    MAX_FIELDIDENTLEN(0),
    MAX_VARCHAR(0),
    ESCAPABLE(""),
    ESCAPE_SYMBOL(0),
    TYPE_NUMBER(""),
    TYPE_LONG(""),
    TYPE_FLOAT(""),
    TYPE_VARCHAR(""),
    TYPE_CLOB(""),
    TYPE_DATE(""),
    TYPE_TIME("") {};
  // Операция присваивания запрещена
  CDatabase& operator=(CDatabase& other);
  std::vector<std::string> warning_msgs;
  std::string error_msg;
};

#endif // __DATABASE_STRUCT__
