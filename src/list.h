/*
  list.h
  Copyright (c) Takashige Sugie <takashige@users.sourceforge.net>, All rights reserved.
  Last update: <2007-11-10 14:41:56>
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

#ifndef __TAKA_LIST_H__
#define __TAKA_LIST_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct tlist {
    struct tlist *next;
    struct tlist *prev;
    void *data;
  } tlist_t;

  static inline void tlist_add(tlist_t *list, tlist_t *p)
  {
    if (list) {
      p->next = list->next;
      list->next = p;
      p->prev = list;
      if (p->next) p->next->prev = p;
    } else {
      list = p;
      list->prev = NULL;
      list->next = NULL;
    }
  }

  static inline void tlist_del(tlist_t *p)
  {
    if (p->prev) p->prev->next = p->next;
    if (p->next) p->next->prev = p->prev;
    p->next = NULL;
    p->prev = NULL;
  }

#define	tlist_for(p, list)		for (p = list; p; p = p->next)
#define	tlist_for_reverse(p, list)	for (p = list; p; p = p->prev)

  static inline tlist_t * tlist_free(tlist_t *p, void (*f)(void *))
  {
    if (!p) return (NULL);
    tlist_t *t = NULL;
    f ? (*f)(p->data) : free(p->data);
    p->data = NULL;
    if (p->prev) t = p->prev;
    if (p->next) t = p->next;
    tlist_del(p);
    free(p);
    return (t);
  }

  static inline void tlist_free_forward(tlist_t *p, void (*f)(void *))
  {
    if (!p) return;
    tlist_t *q = p->next, *r;
    while (q) {
      r = q->next;
      f ? (*f)(q->data) : free(q->data);
      q->next = NULL;
      q->prev = NULL;
      q->data = NULL;
      free(q);
      q = r;
    }
    p->next = NULL;
  }

  static inline void tlist_free_backward(tlist_t *p, void (*f)(void *))
  {
    if (!p) return;
    tlist_t *q = p->prev, *r;
    while (q) {
      r = q->prev;
      f ? (*f)(q->data) : free(q->data);
      q->next = NULL;
      q->prev = NULL;
      q->data = NULL;
      free(q);
      q = r;
    }
    p->prev = NULL;
  }

  static inline void tlist_free_all(tlist_t *p, void (*f)(void *))
  {
    tlist_free_backward(p, f);
    tlist_free_forward(p, f);
    tlist_free(p, f);
  }

  static inline tlist_t * tlist_first(tlist_t *p)
  {
    if (!p) return (NULL);
    while (p->prev) p = p->prev;
    return (p);
  }

  static inline tlist_t * tlist_last(tlist_t *p)
  {
    if (!p) return (NULL);
    while (p->next) p = p->next;
    return (p);
  }

#ifdef __cplusplus
}
#endif

#endif // __TAKA_LIST_H__
