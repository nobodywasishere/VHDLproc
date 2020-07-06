/*
  string.c
  Copyright (c) Takashige Sugie <takashige@users.sourceforge.net>, All rights reserved.
  Last update: <2010-06-16 16:15:45>
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

#ifdef HAVE_CONFIG_H
# include "../../config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "taka/string.h"

void taka_tstring_one_tab_space(char *s)
{
  char *p, *q;
  int f = FALSE;

  for (p = s; *p == ' ' || *p == '\t'; p++);
  for (q = s; *p != '\0'; p++) {
    if (*p == ' ' || *p == '\t') {
      if (!f) *q++ = ' ';
      f = TRUE;
    } else {
      *q++ = *p;
      f = FALSE;
    }
  }
  if (q != s) {
    if (*(q - 1) == ' ') {
      *(q - 1) = '\0';
    } else {
      *q = '\0';
    }
  }
}

void taka_tstring_replace_str(char **s, size_t *n, const char *target, const char *ns)
{
  char *p, *q, *r, *o = NULL;
  size_t l, m;
  size_t tl = strlen(target);
  size_t nsl = strlen(ns);

  if (tl < nsl) {
    o = *s;
    l = strlen(*s);
    m = l / tl * (nsl - tl);
    if (*n - (l + 1) < m) {
      *s = (char *)malloc(sizeof (char) * (l + m + 1));
      if (!*s) { *s = o; return; }
      *n = l + m + 1;
    } else {
      *s = (char *)malloc(*n);
      if (!*s) { *s = o; return; }
    }
    p = *s;
    r = o;
  } else {
    p = *s;
    r = *s;
  }

  while ((q = strstr(r, target))) {
    memcpy(p, r, q - r);
    p += q - r;
    memcpy(p, ns, nsl);
    p += nsl;
    r = q + tl;
  }

  strcpy(p, r);
  free(o);
}

void taka_tstring_delete_char(char *s, const char target)
{
  char *p = s, *q = s;

  for (; *p != '\0'; p++) if (*p != target) *q++ = *p;
  *q = '\0';
}

void taka_tstring_decode_escape_sequence(char *str, const char d, const char e)
{
  char *p = str;

  for (; *p != d; p++) {
    if (*p == '\0') {
      *str = '\0';
      return;
    }
  }
  p++;

  for (; *p != d; p++, str++) {
    if (*p == '\0' || *p == d) break;
    if (*p == e) p++;
    *str = *p;
  }
  *str = '\0';
}

void taka_tstring_delete_file_name_from_path(char *str)
{
  char *p = str + strlen(str);

  for (; p != str; p--) {
    if (*p == '/') {
      p[1] = '\0';
      break;
    }
  }
}
