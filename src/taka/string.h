/*
  string.h
  Copyright (c) Takashige Sugie <takashige@users.sourceforge.net>, All rights reserved.
  Last update: <2010-06-16 16:17:37>
*/

/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef	__TAKA_STRING_H__
#define	__TAKA_STRING_H__

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

  void taka_tstring_one_tab_space(char *);
  void taka_tstring_replace_str(char **, size_t *, const char *, const char *);
  void taka_tstring_delete_char(char *, const char);
  void taka_tstring_decode_escape_sequence(char *, const char, const char);
  void taka_tstring_delete_file_name_from_path(char *);

#ifdef __cplusplus
}
#endif

#endif // __TAKA_STRING_H__
