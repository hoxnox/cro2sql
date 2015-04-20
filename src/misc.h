/******************************************************************************
ProjectName: -
FileName: misc.h 20080116
Subj: Различные функции
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

#ifndef __MISC_H__
#define __MISC_H__

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cerrno>

/// Получить атрибуты файла операционной системы
int file_mode(std::string);
/// Получить размер файла
size_t file_size(std::string);
/// Преобразовать символы кириллицы из CP1251 кодировки в CP866
std::string CP1251_CP866(std::string);
bool is_printable(char);
bool is_alpha(char);
bool is_lat_alpha(char symbol);
bool is_digit(char symbol);
bool is_alpha(std::string str);
bool is_digit(std::string str);
bool is_printable(std::string str);
bool is_operand(std::string str);
#endif //__MISC_H__

