/******************************************************************************
ProjectName: getopt
FileName: getopt.hpp  2008.12.01
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

#ifndef __GETOPT__
#define __GETOPT__ 1

#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

/**
* \class CGetoptManager
* \brief Менеджер разборщика коммандной строки.
*
* Класс-одиночка позволяет быстро осуществлять разбор коммандной строки, проверять на наличие ошибок, выдавать в стандартный поток вывода сообщения.
* Использование (в квадратных скобках указаны необязательные шаги):
*   1. Регистрируем все возможные опции коммандной строки с помощью функции-члена register_option.
*   2. В цикле (как в GNU getopt) вызываем функцию-член getopt до тех пор, пока она не возвратит -1, и обрабатываем возвращаемое этой функцией значение и значение переменной option
*
**/
class COptionsManager
{
public:

  /// Описывает имеет ли опция агрумент. если да, то, какого вида этот аргумент.
  typedef enum{
    NO_ARGUMENT,        //<! Опция не имеет аргумента.
    REQUIRED_ARGUMENT,  //<! Опция требует аргумент.
    OPTIONAL_ARGUMENT   //<! Аргумент опции является необязательным.
  }ARGTYPE;

  /// Регистрирует опцию коммандной строки.
  void register_option(char short_name, string long_name, ARGTYPE argument_type, string description = "");
  /// Регистрирует опцию коммандной строки и сообщает какая переменная будет отвечать за хранение флага (см GNU getopt). Тип int - оставлен как анахронизм.
  void register_option(char short_name, string long_name, bool * flag, string description = "");

  /// Обрабатывает очередной элемент коммандной строки (массива argv[])
  /**
  *  \retval int 0 - getopt усановил зарегистрированный флаг, -1 в случае, если достигнут конец, '?' - если аргумент не является действительной опцией, ':' - если опция не имеет аргумента.
  * Функция обрабатывает следующий за _cursor аргумент. Если аргументов больше нет, getopt вернет -1. Если аргумент есть и он является опцией, которая не была зарегистрирована, функция возвратит '?', если опция была зарегистрирована, функция выставит итератор COptionsManager.option на найденную в vector опцию.
  *   Если у опции должен быть аргумент, но отсутствует, getopt вернет ':'.
  *   Если к опции была привязана переменная, отвечающая за хранение флага, флаг выставляется и getopt возвращает 0
  *   Если getopt нашел зарегистрированную ранее опцию без флага возвращает 1;
  **/
  int getopt(int argc, const char * argv[]);

  /// Метод получения экземпляра одиночки
  static COptionsManager * Instance();

  /// Возвращает короткое имя зарегистрированной опции, встреченной getopt на данном шаге
  char current_short_name() const;
  /// Возвращает длинное имя зарегистрированной опции, встреченной getopt на данном шаге
  string current_long_name() const;
  /// Возвращает аргументы зарегистрированной опции, встреченной getopt на данном шаге и разобранные из коммандной строки
  string current_arguments() const;

  /// Осуществляет проверку - зарегистрирована ли опция с данным коротким именем
  bool is_registered(char short_name);
  /// Осуществляет проверку - зарегистрирована ли опция с данным длинным
  bool is_registered(string long_name);

  /// Устанавливает POSIX-style обработку опции "W" в true
  /**
  * Стандарт POSIX резервирует опцию -W для специфических для производителя возможностей. Поэтому по определению -W непереносимо между различными системами.
  * Если posix_W выставлен в true, функция getopt рассматривает -Wlongopt как --longopt. Соответственно -Wfile=myfile рассматривается как --file=myfile. Поумолчанию posix_W выставлен в false и -W - рассматривается как обычная опция.
  **/
  void set_posix_W();

  /// Очищает объект, как будто не запускали getopt
  /**
  * Все зарегистрированные опции остаются. Остается включенным (если был) posix_W. Символ разделителя также не меняется.
  * \warning Функция не сбрасывает установленные getopt флаги. Использование этой функции нежелательно, она была реализована для тестирования
  **/
  void reset();

  /// Возвращает номер argv[], обработанный getopt последним
  int cursor() const;

private:
  COptionsManager();
  /// Структура описывает опцию коммандной строки
  typedef struct{
    char short_name;                //<! Короткое имя опции
    string long_name;               //<! Длинное имя опции
    string description;             //<! Описание опции
    bool * flag;                    //<! Ссылка на флаг
    ARGTYPE argument_type;          //<! Тип аргументов опции
  }TOption;
  vector<TOption>::iterator option; //<! Указатель на обрабатываемую в данный момент функцией getopt опцию
  vector<TOption> options;          //<! Вектор содержит все зарегистрированные опции
  string _arguments;                //<! Аргументы текущей опции
  bool _posix_W;                    //<! Флаг - индикатор разбора опции -W (см set_posix_W())
  int _cursor;                      //<! Номер текущего аргумента
  static COptionsManager * _instance;
  void _test_set_names(const char short_name, const string &long_name, const string &description, TOption &option);
  bool _test_unique(const char short_name);
  bool _test_long_optname(string str);
  bool _test_unique(const string &long_name);
  string _reminder;                 //<! Формально, getopt работает не с argv, а с _reminder
  string _all_short_names;          //<! При регистрации опции в конец этой строки добавляется ее короткое имя. Строка служит для быстрого поиска зарегистрирована ли опция с данным коротким именем.
  vector<string> _all_long_names;   //<! При регистрации опции в конец этого вектора добавляется ее длинное имя. Вектор служит для быстрого поиска зарегистрирована ли опция с данным длинным именем.
};


/**
*  \class getopt_error
*  Основной класс, выбрасываемый как исключение.
**/
class getopt_error : public logic_error
{
public:
  getopt_error(string);
};

/**
*  \class registration_error
*  Отвечает за исключения, генерируемые при регистрации опций.
**/
class registration_error : public getopt_error
{
public:
  registration_error(string);
};

/**
*  \class parce_error
* Отвечает за исключения, генерируемые при разборе коммандной строки
**/
class parce_error : public getopt_error
{
public:
  parce_error(string);
};

#endif // __GETOPT__
