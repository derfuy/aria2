/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _BASE64_H_
#define _BASE64_H_
#include <string>
#include "common.h"
using namespace std;

class Base64
{
private:
  static void part_encode(const unsigned char* sub, int subLength,
			  unsigned char* buf);

  static string part_encode(const string& subplain);
  static string part_decode(const string& subCrypted);
  static char getValue(char ch);
public:
  static string encode(const string& plain);
  // caller must deallocate the memory used by result.
  static void encode(const unsigned char* src, int srcLength,
		     unsigned char*& result, int& resultLength);
  static string decode(const string& crypted);
  // caller must deallocate the memory used by result.
  static void decode(const unsigned char* src, int srcLength,
		     unsigned char*& result, int& resultLength);
};

#endif // _BASE64_H_
