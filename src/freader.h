/******************************************************************************
ProjectName: freader
FileName: freader.h 20081203
Subj: "Докрутка" fstream для возможности пролиcтывать файл до определенного символа.
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

#ifndef __FREADER_H__
#define __FREADER_H__

#include <fstream>
#include <string>

class CFileReader : public std::ifstream
{
public:
  CFileReader();
  CFileReader(std::string);
  // просматривает поток до тех пор, пока не встретит str или end_symbol. Как только попадается str, заканчивает пролистываение потока, возвращается true. Если встречается end_symbol раньше или по какой-то иной причине чтение из потока прекращается, возвращая false.
  bool skip_until(const std::string &str, const char end_symbol = 0);
  // если следующая полученная из потока часть равняется str, то возвращает true, false - если не равняется или встречен end_symbol
  bool get_is_equal(const std::string &str, const char end_symbol = 0);
  // Читает поток ввода до тех пор пока не встретит delim или end_symbol
  std::string get_until(const std::string &delim, char end_symbol = 0);
};

#endif //__FREADER_H__
