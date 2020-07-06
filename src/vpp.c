/*
  vpp.c [ vpp / VHDL PreProcessor ]
  Copyright (c) Takashige Sugie <takashige@users.sourceforge.net>, All rights reserved.
  Last update: <2018-05-15 13:21:41>
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
# include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <iconv.h>
#include <errno.h>
#include "taka/list.h"
#include "taka/string.h"
#include "vpp.h"

#ifndef LINE_BUFFER_SIZE
#define	LINE_BUFFER_SIZE	4096
#endif
#define	PPS			'#'
#define	COMMENTS_START		"/*"
#define	COMMENTS_END		"*/"
#define	COMMENT_CHAR		'-'
#define	SD			'"'	// sequence delimiter
#define	ES			'\\'	// escape sequence

static const char *taka_tvpp_pp_keywords[] = {
  "include",
  "define",
  "rand",
  "undef",
  "ifdef",
  "ifndef",
  "elif",
  "else",
  "endif",
  "for",
  "endfor",
  "message",
  ""
};

typedef struct taka_tvpp_str_pair {
  char *	org;
  char *	chg;
} taka_tvpp_str_pair_t;

typedef struct taka_tvpp_for_pos {
  long		begin;
  long		end;
  int		loop;
  int		flag;
} taka_tvpp_for_pos_t;

typedef struct taka_tvpp_data {
  char *		o_line_str;
  char *		n_line_str;
  unsigned		line_num;
  int			comment;
  char *		keyword;
  char *		label;
  char *		string;
  char *		including;
  tlist_t *		defined;
  tlist_t *		replace;
  tlist_t *		rand;
  unsigned		ifdepth;
  unsigned long		ifstat;
  unsigned long		elsestat;
  taka_tvpp_for_pos_t	forstat;
  int			error;
} taka_tvpp_data_t;

//const char * taka_tvpp_version = "2.0.3";

static void taka_tvpp_tlist_str_pair_free(void *p)
{
  if (!p) return;
  free(((taka_tvpp_str_pair_t *)p)->org);
  free(((taka_tvpp_str_pair_t *)p)->chg);
  free(p);
}

inline static void taka_tvpp_certification(FILE *fp, const char *orig_file, const char cc)
{
  time_t t;
  char rp[FILENAME_MAX];

  fprintf(fp, "\n\n%c%c\n", cc, cc);
  fprintf(fp, "%c%c Preprocessed with vpp-%s\n", cc, cc, VERSION);
  realpath(orig_file, rp);
  fprintf(fp, "%c%c for %s\n", cc, cc, rp);
  fprintf(fp, "%c%c by %s\n", cc, cc, secure_getenv("USER"));
  //fprintf(fp, "%c%c on %s\n", cc, cc, secure_getenv("HOSTNAME"));
  gethostname(rp, sizeof (rp));
  fprintf(fp, "%c%c on %s\n", cc, cc, rp);
  time(&t);
  fprintf(fp, "%c%c at %s", cc, cc, ctime(&t));
  fprintf(fp, "%c%c\n", cc, cc);
}

static inline char * taka_tvpp_check_symbol(taka_tvpp_data_t *d, FILE *cerr)
{
  int i;
  char *p;
  tlist_t *l;
  taka_tvpp_str_pair_t *sp;

  // normalize
  strcpy(d->n_line_str, d->o_line_str);
  taka_tstring_one_tab_space(d->n_line_str);

  // check comments
  if (*d->n_line_str != PPS && (p = strstr(d->o_line_str, COMMENTS_END))) {
    d->comment = FALSE;
    return (p);
  }
  if (d->comment) return (NULL);
  if (*d->n_line_str != PPS && (p = strstr(d->o_line_str, COMMENTS_START))) {
    d->comment = TRUE;
    return (p);
  }

  // preprocessor initialize
  if (*d->n_line_str != PPS) return (NULL);
  *d->keyword = '\0';
  *d->label = '\0';
  *d->string = '\0';
  p = d->n_line_str;
  for (++p; *p == ' ' || *p == '\t'; p++);

  // get keyword and label
  sscanf(p, "%s %s", d->keyword, d->label);
  // get string
  if (p[strlen(d->keyword) + 1 + strlen(d->label)] != '\0') {
    strcpy(d->string, p + strlen(d->keyword) + 1 + strlen(d->label) + 1);
  }
  // get keyword index
  for (i = 0; taka_tvpp_pp_keywords[i][0] != '\0'; i++) {
    if (!strcmp(d->keyword, taka_tvpp_pp_keywords[i])) break;
  }

  switch (i) {
  case 0: // include
    if (d->ifstat) break;
    strcat(d->including, d->o_line_str);
    taka_tstring_decode_escape_sequence(d->including, SD, ES);
    if (*d->including == '\0') {
      if (cerr) fprintf(cerr, "No include file name is found. (Can't find a delimiter '%c')\n", SD);
    }
    break;
  case 1: // define
    if (d->ifstat) break;
    if (*d->string == '\0') { // define
      if (!(l = (tlist_t *)malloc(sizeof (tlist_t)))) break;
      l->data = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);
      strcpy(l->data, d->label);
      tlist_add(d->defined, l);
      d->defined = l;
    } else { // replace
      if (!(l = (tlist_t *)malloc(sizeof (tlist_t)))) break;
      if (!(sp = (taka_tvpp_str_pair_t *)malloc(sizeof (taka_tvpp_str_pair_t)))) {free(l);break;}
      sp->org = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);
      sp->chg = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);
      strcpy(sp->org, d->label);
      strcpy(sp->chg, d->string);
      taka_tstring_decode_escape_sequence(sp->chg, SD, ES);
      if (*sp->chg == '\0') {
	if (cerr) fprintf(cerr, "No replacing string is found. (Can't find a delimiter '%c')\n", SD);
	free(sp->chg);
	free(sp->org);
	free(sp);
	free(l);
	break;
      }
      l->data = sp;
      tlist_add(d->replace, l);
      d->replace = l;
    }
    break;
  case 2: // rand
    if (d->ifstat) break;
    if (atoi(d->string + 1) < 1) {
      if (cerr) fprintf(cerr, "Invalid number for #rand was found at line %d.\n", d->line_num);
      break;
    }
    switch (*d->string) {
    case 'B': // bin
    case 'b':
    case 'D': // dec
    case 'd':
    case 'H': // hex
    case 'h':
    case 'A': // alphabet
    case 'a':
      if (!(l = (tlist_t *)malloc(sizeof (tlist_t)))) break;
      if (!(sp = (taka_tvpp_str_pair_t *)malloc(sizeof (taka_tvpp_str_pair_t)))) {free(l);break;}
      sp->org = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);
      sp->chg = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);
      strcpy(sp->org, d->label);
      strcpy(sp->chg, d->string);
      l->data = sp;
      tlist_add(d->rand, l);
      d->rand = l;
      break;
    default:
      if (cerr) fprintf(cerr, "Unknown radix for #rand was found at line %d.\n", d->line_num);
      break;
    }
    break;
  case 3: // undef
    if (d->ifstat) break;
    if (d->replace) {
      tlist_for_reverse(l, d->replace) {
	if (!strcmp(((taka_tvpp_str_pair_t *)l->data)->org, d->label)) {
	  d->replace = tlist_free(l, taka_tvpp_tlist_str_pair_free);
	}
      }
      d->replace = tlist_last(d->replace);
    }
    if (d->defined) {
      tlist_for_reverse(l, d->defined) {
	if (!strcmp(l->data, d->label)) {
	  d->defined = tlist_free(l, NULL);
	}
      }
      d->defined = tlist_last(d->defined);
    }
    if (d->rand) {
      tlist_for_reverse(l, d->rand) {
	if (!strcmp(((taka_tvpp_str_pair_t *)l->data)->org, d->label)) {
	  d->rand = tlist_free(l, taka_tvpp_tlist_str_pair_free);
	}
      }
      d->rand = tlist_last(d->rand);
    }
    break;
  case 4: // ifdef
  case 5: // ifndef
ifdef_label:
    if (!d->label) {
      if (cerr) fprintf(cerr, "No label was found at line %d.\n", d->line_num);
      break;
    }
    if (8 * sizeof (unsigned long) <= ++d->ifdepth) {
      if (cerr) fprintf(cerr, "Too many nests were found at line %d.\n", d->line_num);
      break;
    }
    d->ifstat |= (0x1 << d->ifdepth);
    tlist_for_reverse(l, d->defined) {
      if (!strcmp((char *)l->data, d->label)) {
	d->ifstat &= ~(0x1 << d->ifdepth);
	break;
      }
    }
    if (i == 5) d->ifstat = (d->ifstat & ~(0x1 << d->ifdepth)) | (~d->ifstat & (0x1 << d->ifdepth));
    break;
  case 6: // elif
  case 7: // else
    if (!d->ifdepth || (d->elsestat & (0x1 << d->ifdepth))) if (cerr) fprintf(cerr, "Invalid #else was found at line %d.\n", d->line_num);
    d->elsestat |= (0x1 << d->ifdepth);
    d->ifstat = (d->ifstat & ~(0x1 << d->ifdepth)) | (~d->ifstat & (0x1 << d->ifdepth));
    if (i == 6) {
      i = 4;
      goto ifdef_label;
    }
    break;
  case 8: // endif
    if (!d->ifdepth) if (cerr) fprintf(cerr, "Invalid #endif was found at line %d.\n", d->line_num);
    d->elsestat &= ~(0x1 << d->ifdepth);
    d->ifstat &= ~(0x1 << d->ifdepth);
    d->ifdepth--;
    break;
  case 9: // for
    if (d->ifstat) break;
    if (d->forstat.loop) {
      if (cerr) fprintf(cerr, "Invalid #for was found at line %d.\n", d->line_num);
      break;
    }
    d->forstat.flag = 0x1;
    d->forstat.loop = atoi(d->label);
    if (d->forstat.loop < 1) {
      if (cerr) fprintf(cerr, "Invalid loop count for #for was found at line %d.\n", d->line_num);
      d->forstat.loop = 1;
    }
    break;
  case 10: // endfor
    if (d->ifstat) break;
    if (!d->forstat.loop) {
      if (cerr) fprintf(cerr, "Invalid #endfor was found at line %d.\n", d->line_num);
      break;
    }
    d->forstat.flag = 0x2;
    break;
  case 11: // message
    if (d->ifstat) break;
    if (cerr) {
      strcpy(d->n_line_str, d->o_line_str);
      taka_tstring_decode_escape_sequence(d->n_line_str, SD, ES);
      if (*d->n_line_str == '\0') {
	if (cerr) fprintf(cerr, "No message string is found. (Can't find a delimiter '%c')\n", SD);
      }
      if (!strcmp(d->label, VPP_MES_DEBUG)) printf("vpp debug[%d]: ", d->line_num);
      else if (!strcmp(d->label, VPP_MES_INFO)) printf("vpp info[%d]: ", d->line_num);
      else if (!strcmp(d->label, VPP_MES_NOTICE)) printf("vpp notice[%d]: ", d->line_num);
      else if (!strcmp(d->label, VPP_MES_WARN)) printf("vpp warn[%d]: ", d->line_num);
      else if (!strcmp(d->label, VPP_MES_ERR)) {printf("vpp error[%d]: ", d->line_num);d->error = -EVPPMESERR;}
      else if (!strcmp(d->label, VPP_MES_CRIT)) {printf("vpp critical[%d]: ", d->line_num);d->error = -EVPPMESCRIT;}
      else if (!strcmp(d->label, VPP_MES_ALERT)) {printf("vpp alert[%d]: ", d->line_num);d->error = -EVPPMESALERT;}
      else if (!strcmp(d->label, VPP_MES_EMERG)) {printf("vpp emergency[%d]: ", d->line_num);d->error = -EVPPMESEMERG;}
      printf("%s\n", d->n_line_str);
    }
    break;
  default:
    if (cerr) fprintf(cerr, "Unknown preprocess symbol was found at line %d.\n", d->line_num);
    break;
  }

  return (d->n_line_str);
}

int taka_tvpp_convert(const char *orig_file, const char *conv_file, tlist_t *def, const char cc, FILE *cerr)
{
  FILE *of, *cf, *incf;
  taka_tvpp_data_t d;
  int cs = FALSE; // Comment Status
  char *p = NULL;
  tlist_t *l;
  size_t n = LINE_BUFFER_SIZE;
  char *pRand, *r;
  int b, rn;
  int f;
  long forpos = -1;
  iconv_t iconv_handler;
  const size_t const_LBS = sizeof (char) * LINE_BUFFER_SIZE;
  char *ostr, *nstr, *inc_obuf, *inc_nbuf;
  size_t olen, nlen;

  srand((unsigned)time(NULL));

  iconv_handler = iconv_open("UTF-8", "SHIFT-JIS");

  memset(&d, 0, sizeof (d));
  d.o_line_str = (char *)malloc(const_LBS);
  d.n_line_str = (char *)malloc(const_LBS);
  d.keyword = (char *)malloc(const_LBS);
  d.label = (char *)malloc(const_LBS);
  d.string = (char *)malloc(const_LBS);
  d.including = (char *)malloc(FILENAME_MAX);
  *d.including = '\0';
  d.defined = def;
  inc_obuf = (char *)malloc(const_LBS);
  inc_nbuf = (char *)malloc(const_LBS);

  of = fopen(orig_file, "r");
  if (!of) {
    if (cerr) fprintf(cerr, "Can't open the original file: %s.\n", orig_file);
    return (-EVPPORIG);
  }
  cf = fopen(conv_file, "w");
  if (!cf) {
    fclose(of);
    if (cerr) fprintf(cerr, "Can't open a converted file: %s.\n", conv_file);
    return (-EVPPCONV);
  }

  for (;;) {
    if (!fgets(d.o_line_str, const_LBS, of)) break;
    ostr = d.o_line_str;
    olen = strlen(d.o_line_str);
    nstr = d.n_line_str;
    nlen = const_LBS - 1;
    //printf("%s", d.o_line_str);
    while (0 < olen) iconv(iconv_handler, &ostr, &olen, &nstr, &nlen);
    *nstr = '\0';
    //printf("iconv(%d): %s\n", errno, strerror(errno));
    //printf("%s", d.n_line_str);
    nstr = strstr(d.n_line_str, "\r\n");
    if (nstr) strcpy(nstr, "\n");
    strcpy(d.o_line_str, d.n_line_str);
    if (!(d.forstat.loop && d.forstat.begin != d.forstat.end)) d.line_num++;
    if (!feof(of) && !(p = strchr(d.o_line_str, '\n'))) {
      if (cerr) fprintf(cerr, "Too long string at line %i.\n", d.line_num);
      break;
    } else {
      *p = '\0';
    }

    p = taka_tvpp_check_symbol(&d, cerr);

    // comment out
    if (d.comment) {
      if (cs) {
	fputc(cc, cf);
	fputc(cc, cf);
      } else { // comment start
	p[0] = cc;
	p[1] = cc;
      }
    } else {
      if (cs) {
	fputc(cc, cf);
	fputc(cc, cf);
	if (p[2] != '\0') { // p is pointed to "*/"
	  //p[0] = '\n';
	  p[1] = '\n';
	}
      } else {
	if (p || d.ifstat) {
	  fputc(cc, cf);
	  fputc(cc, cf);
	}
      }
    }
    cs = d.comment;

    if (!cs && !d.ifstat && *d.n_line_str != PPS) {
      // replace
      tlist_for_reverse(l, d.replace) {
	taka_tstring_replace_str(&d.o_line_str, &n, ((taka_tvpp_str_pair_t *)l->data)->org, ((taka_tvpp_str_pair_t *)l->data)->chg);
      }

      // rand
      tlist_for_reverse(l, d.rand) {
	rn = atoi(((taka_tvpp_str_pair_t *)l->data)->chg + 1);
	pRand = (char *)malloc(sizeof (char) * (rn + 4));
	switch (*((taka_tvpp_str_pair_t *)l->data)->chg) {
	case 'B':
	case 'b':
	  f = (rn == 1) ? TRUE : FALSE;
	  r = pRand;
	  *r++ = f ? '\'' : '"';
	  for (; 0 < rn; rn--) *r++ = (rand() % 2) ? '1' : '0';
	  *r++ = f ? '\'' : '"';
	  *r = '\0';
	  break;
	case 'D':
	case 'd':
	  r = pRand;
	  *r++ = (char)((rand() % 9) + '1');
	  for (rn--; 0 < rn; rn--) *r++ = (char)((rand() % 10) + '0');
	  *r = '\0';
	  break;
	case 'H':
	case 'h':
	  r = pRand;
	  *r++ = 'X';
	  *r++ = '"';
	  for (; 0 < rn; rn--) {
	    b = rand() % 16;
	    if (b < 10) {
	      *r++ = (char)(b + '0');
	    } else {
	      *r++ = (char)(b + '7'); // '7' = 'A' - 10
	    }
	  }
	  *r++ = '"';
	  *r = '\0';
	  break;
	case 'A':
	case 'a':
	  for (r = pRand; 0 < rn; rn--) *r++ = (char)((rand() % 26) + 'A');
	  *r = '\0';
	  break;
	}

	taka_tstring_replace_str(&d.o_line_str, &n, ((taka_tvpp_str_pair_t *)l->data)->org, pRand);

	free(pRand);
      }
    }

    if (!(d.forstat.flag & 0x2)) fputs(d.o_line_str, cf);
    if (!feof(of)) fputc('\n', cf);

    // include
    if (*d.including != '\0') {
      strcpy(d.string, orig_file);
      taka_tstring_delete_file_name_from_path(d.string);
      strcat(d.string, d.including);
      incf = fopen(d.string, "r");
      if (incf) {
	while (fgets(inc_obuf, const_LBS, incf)) {
	  ostr = inc_obuf;
	  olen = strlen(inc_obuf);
	  nstr = inc_nbuf;
	  nlen = const_LBS - 1;
	  while (0 < olen) iconv(iconv_handler, &ostr, &olen, &nstr, &nlen);
	  *nstr = '\0';
	  nstr = strstr(inc_nbuf, "\r\n");
	  if (nstr) strcpy(nstr, "\n");
	  fputs(inc_nbuf, cf);
	}
	fclose(incf);
      } else {
	if (cerr) fprintf(cerr, "Can't open %s.\n", d.string);
      }
      *d.including = '\0';
    }

    // for
    if (d.forstat.loop) {
      if (d.forstat.flag & 0x1) { // #for
	d.forstat.begin = ftell(of);
	d.forstat.end = d.forstat.begin;
      }
      if (d.forstat.flag & 0x2) { // #endfor
	d.forstat.end = ftell(of);
      }
      if (!(d.forstat.flag & 0x1)) forpos = ftell(of);
      if (forpos == d.forstat.end) {
	if (--d.forstat.loop) {
	  fseek(of, d.forstat.begin, SEEK_SET);
	} else {
#ifdef HOST_CYGWIN
	  fseek(cf, -2, SEEK_CUR);
#else
	  fseek(cf, -1, SEEK_CUR);
#endif
	  fputs(d.o_line_str, cf);
	  fputc('\n', cf);
	}
      }
      d.forstat.flag = 0;
    }
  }

  taka_tvpp_certification(cf, orig_file, cc);

  fclose(cf);
  fclose(of);

  free(inc_nbuf);
  free(inc_obuf);
  tlist_free_all(d.rand, taka_tvpp_tlist_str_pair_free);
  tlist_free_all(d.replace, taka_tvpp_tlist_str_pair_free);
  tlist_free_all(d.defined, NULL);
  free(d.including);
  free(d.string);
  free(d.label);
  free(d.keyword);
  free(d.n_line_str);
  free(d.o_line_str);

  iconv_close(iconv_handler);

  if (d.ifdepth) {
    if (cerr) fprintf(cerr, "#ifdef is NOT closed by #endif.\n");
    return (-EVPPNOENDIF);
  }
  if (d.forstat.loop) {
    if (cerr) fprintf(cerr, "#for is NOT closed by #endfor.\n");
    return (-EVPPNOENDFOR);
  }

  return (d.error);
}

tlist_t * taka_tvpp_define_label(tlist_t *list, const char *label)
{
  tlist_t *l;

  if (!(l = (tlist_t *)malloc(sizeof (tlist_t)))) return (NULL);
  l->data = malloc(sizeof (char) * LINE_BUFFER_SIZE);
  strcpy(l->data, label);
  tlist_add(list, l);

  return (l);
}

inline static int taka_tvpp_create_module_template(const char *orig_file, FILE *cerr)
{
  FILE *of, *cf;
  char *name, *line, *p, *q;

  of = fopen(orig_file, "r");
  if (!of) {
    if (cerr) fprintf(cerr, "Can't open the original file: %s.\n", orig_file);
    return (-EVPPMTORIG);
  }
  cf = fopen(VPP_TEMPLATE_MODULE, "w");
  if (!cf) {
    fclose(of);
    if (cerr) fprintf(cerr, "Can't open a converted file: %s.\n", VPP_TEMPLATE_MODULE);
    return (-EVPPMTCONV);
  }
  name = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);
  line = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);

  fprintf(cf, "library ieee;\n");
  fprintf(cf, "use ieee.std_logic_1164.all;\n");
  fprintf(cf, "\n");
  fscanf(of, "%s %s\n", line, name);
  if (!strcmp(line, VPP_TEMPLATE_ENTITY) || !strcmp(line, VPP_TEMPLATE_COMPONENT)) fprintf(cf, "entity %s is\n", name);
  fscanf(of, "%s\n", line);
  if (!strcmp(line, VPP_TEMPLATE_GENERIC)) {
    fprintf(cf, "\tgeneric (\n");
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      if (!strcmp(p, VPP_TEMPLATE_PORT)) break;
      fprintf(cf, "\t\t%s:\t\t\t", p);
      p = q + 1;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s := ", p);
      p = q + 1;
      for (q = p; *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s;\n", p);
    }
#ifdef HOST_CYGWIN
    fseek(cf, -3, SEEK_CUR);
#else
    fseek(cf, -2, SEEK_CUR);
#endif
    fprintf(cf, "\n\t);\n");
  }
  if (!strcmp(line, VPP_TEMPLATE_PORT)) {
    fprintf(cf, "\tport (\n");
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "\t\t%s:\t\t\t", p);
      p = q + 1;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s ", p);
      p = q + 1;
      for (q = p; *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s;\n", p);
    }
#ifdef HOST_CYGWIN
    fseek(cf, -3, SEEK_CUR);
#else
    fseek(cf, -2, SEEK_CUR);
#endif
    fprintf(cf, "\n\t);\n");
  }
  fprintf(cf, "end entity %s;\n", name);
  fprintf(cf, "\n");
  fprintf(cf, "architecture rtl of %s is\n", name);
  fprintf(cf, "\n");
  fprintf(cf, "begin\n");
  fprintf(cf, "\n");
  fprintf(cf, "process(clock, reset)\n");
  fprintf(cf, "begin\n");
  fprintf(cf, "if (reset = '1') then\n");
  fprintf(cf, "\n");
  fprintf(cf, "elsif (rising_edge(clock)) then\n");
  fprintf(cf, "\n");
  fprintf(cf, "end if;\n");
  fprintf(cf, "end process;\n");
  fprintf(cf, "\n");
  fprintf(cf, "end rtl;\n");

  free(line);
  free(name);
  fclose(cf);
  fclose(of);

  return (0);
}

inline static int taka_tvpp_create_package_template(const char *orig_file, FILE *cerr)
{
  FILE *of, *cf;
  char *name, *line, *p, *q;

  of = fopen(orig_file, "r");
  if (!of) {
    if (cerr) fprintf(cerr, "Can't open the original file: %s.\n", orig_file);
    return (-EVPPMTORIG);
  }
  cf = fopen(VPP_TEMPLATE_PACKAGE, "w");
  if (!cf) {
    fclose(of);
    if (cerr) fprintf(cerr, "Can't open a converted file: %s.\n", VPP_TEMPLATE_PACKAGE);
    return (-EVPPMTCONV);
  }
  name = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);
  line = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);

  fprintf(cf, "library ieee;\n");
  fprintf(cf, "use ieee.std_logic_1164.all;\n");
  fprintf(cf, "\n");
  fscanf(of, "%s %s\n", line, name);
  if (!strcmp(line, VPP_TEMPLATE_ENTITY) || !strcmp(line, VPP_TEMPLATE_COMPONENT)) fprintf(cf, "component %s\n", name);
  fscanf(of, "%s\n", line);
  if (!strcmp(line, VPP_TEMPLATE_GENERIC)) {
    fprintf(cf, "\tgeneric (\n");
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      if (!strcmp(p, VPP_TEMPLATE_PORT)) break;
      fprintf(cf, "\t\t%s:\t\t\t", p);
      p = q + 1;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s := ", p);
      p = q + 1;
      for (q = p; *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s;\n", p);
    }
#ifdef HOST_CYGWIN
    fseek(cf, -3, SEEK_CUR);
#else
    fseek(cf, -2, SEEK_CUR);
#endif
    fprintf(cf, "\n\t);\n");
  }
  if (!strcmp(line, VPP_TEMPLATE_PORT)) {
    fprintf(cf, "\tport (\n");
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "\t\t%s:\t\t\t", p);
      p = q + 1;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s ", p);
      p = q + 1;
      for (q = p; *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s;\n", p);
    }
#ifdef HOST_CYGWIN
    fseek(cf, -3, SEEK_CUR);
#else
    fseek(cf, -2, SEEK_CUR);
#endif
    fprintf(cf, "\n\t);\n");
  }
  fprintf(cf, "end component;\n");

  free(line);
  free(name);
  fclose(cf);
  fclose(of);

  return (0);
}

inline static int taka_tvpp_create_simulation_template(const char *orig_file, const char cc, FILE *cerr)
{
  FILE *of, *cf;
  char *name, *line, *p, *q;

  of = fopen(orig_file, "r");
  if (!of) {
    if (cerr) fprintf(cerr, "Can't open the original file: %s.\n", orig_file);
    return (-EVPPMTORIG);
  }
  cf = fopen(VPP_TEMPLATE_SIMULATION, "w");
  if (!cf) {
    fclose(of);
    if (cerr) fprintf(cerr, "Can't open a converted file: %s.\n", VPP_TEMPLATE_SIMULATION);
    return (-EVPPMTCONV);
  }
  name = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);
  line = (char *)malloc(sizeof (char) * LINE_BUFFER_SIZE);

  fprintf(cf, "library ieee, modelsim_lib;\n");
  fprintf(cf, "use ieee.std_logic_1164.all;\n");
  fprintf(cf, "use modelsim_lib.util.all;\n");
  fprintf(cf, "\n");
  fscanf(of, "%s %s\n", line, name);
  if (!strcmp(line, VPP_TEMPLATE_ENTITY) || !strcmp(line, VPP_TEMPLATE_COMPONENT)) fprintf(cf, "entity tb is\n");
  fscanf(of, "%s\n", line);
  if (!strcmp(line, VPP_TEMPLATE_GENERIC)) {
    fprintf(cf, "\tgeneric (\n");
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      if (!strcmp(p, VPP_TEMPLATE_PORT)) break;
      fprintf(cf, "\t\t%s:\t\t\t", p);
      p = q + 1;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s := ", p);
      p = q + 1;
      for (q = p; *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s;\n", p);
    }
#ifdef HOST_CYGWIN
    fseek(cf, -3, SEEK_CUR);
#else
    fseek(cf, -2, SEEK_CUR);
#endif
    fprintf(cf, "\n\t);\n");
  }
  fprintf(cf, "end tb;\n");
  fprintf(cf, "\n");
  fprintf(cf, "architecture %s_simulation of tb is\n", name);
  fprintf(cf, "\n");
  fprintf(cf, "\t%c%c clock\n", cc, cc);
  fprintf(cf, "\tconstant CLOCK_PERIOD:\t\t\ttime := 4.000 ns;\n");
  fprintf(cf, "\n");
  fprintf(cf, "\t%c%c port\n", cc, cc);
  if (!strcmp(line, VPP_TEMPLATE_PORT)) {
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "\tsignal %s:\t\t\t", p);
      p = q + 1;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      p = q + 1;
      for (q = p; *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "%s;\n", p);
    }
    fprintf(cf, "\n");
  }
  fprintf(cf, "\t%c%c signal spy\n", cc, cc);
  fprintf(cf, "\tsignal spy_xxxxx:\t\t\tstd_logic;\n");
  fprintf(cf, "\n");
  fprintf(cf, "begin\n");
  fprintf(cf, "\n");

  fprintf(cf, "\tINST: %s\n", name);
  clearerr(of);
  rewind(of);
  fscanf(of, "%s %s\n", line, name);
  fscanf(of, "%s\n", line);
  if (!strcmp(line, VPP_TEMPLATE_GENERIC)) {
    fprintf(cf, "\tgeneric map (\n");
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      if (!strcmp(p, VPP_TEMPLATE_PORT)) break;
      fprintf(cf, "\t\t%s => %s,\n", p, p);
    }
#ifdef HOST_CYGWIN
    fseek(cf, -3, SEEK_CUR);
#else
    fseek(cf, -2, SEEK_CUR);
#endif
    fprintf(cf, "\n\t)\n");
  }
  if (!strcmp(line, VPP_TEMPLATE_PORT)) {
    fprintf(cf, "\tport map (\n");
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      fprintf(cf, "\t\t%s => %s,\n", p, p);
    }
#ifdef HOST_CYGWIN
    fseek(cf, -3, SEEK_CUR);
#else
    fseek(cf, -2, SEEK_CUR);
#endif
    fprintf(cf, "\n\t);\n");
  }
  fprintf(cf, "\n");

  fprintf(cf, "process begin %c%c signal spy\n", cc, cc);
  fprintf(cf, "\tinit_signal_spy(\"/tb/INST/xxxxx\", \"/spy_xxxxx\", 0);\n");
  fprintf(cf, "\n");
  fprintf(cf, "\twait;\n");
  fprintf(cf, "end process;\n");
  fprintf(cf, "\n");

  fprintf(cf, "process begin\n");
  fprintf(cf, "\tclock <= transport '1';\n");
  fprintf(cf, "\twait for CLOCK_PERIOD / 2;\n");
  fprintf(cf, "\tclock <= transport '0';\n");
  fprintf(cf, "\twait for CLOCK_PERIOD / 2;\n");
  fprintf(cf, "end process;\n");
  fprintf(cf, "\n");

  fprintf(cf, "process begin\n");
  clearerr(of);
  rewind(of);
  fscanf(of, "%s %s\n", line, name);
  fscanf(of, "%s\n", line);
  if (!strcmp(line, VPP_TEMPLATE_GENERIC)) {
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      if (!strcmp(p, VPP_TEMPLATE_PORT)) break;
    }
  }
  if (!strcmp(line, VPP_TEMPLATE_PORT)) {
    while (fgets(line, LINE_BUFFER_SIZE, of) != NULL) {
      taka_tstring_one_tab_space(line);
      p = line;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      strcpy(name, p);
      p = q + 1;
      for (q = p; *q != ' ' && *q != '\n' && *q != '\0'; q++);
      if (*q == '\0') continue;
      *q = '\0';
      if (!strcmp(p, VPP_TEMPLATE_IN) || !strcmp(p, VPP_TEMPLATE_INOUT)) fprintf(cf, "\t%s <= transport ;\n", name);
    }
  }
  fprintf(cf, "\n");
  fprintf(cf, "\twait;\n");
  fprintf(cf, "\n");
  fprintf(cf, "end process;\n");
  fprintf(cf, "\n");
  rewind(of);
  fscanf(of, "%s %s\n", line, name);
  fprintf(cf, "end %s_simulation;\n", name);
  fprintf(cf, "\n");

  fprintf(cf, "configuration %s_cfg of tb is\n", name);
  fprintf(cf, "\tfor %s_simulation\n", name);
  fprintf(cf, "\tend for;\n");
  fprintf(cf, "end %s_cfg;\n", name);

  free(line);
  free(name);
  fclose(cf);
  fclose(of);

  return (0);
}

int taka_tvpp_create_template(const char *orig_file, const char cc, FILE *cerr)
{
  int err;

  err = taka_tvpp_create_module_template(orig_file, cerr);
  if (err) return (err);
  //err = taka_tvpp_create_package_template(orig_file, cerr);
  //if (err) return (err);
  err = taka_tvpp_create_simulation_template(orig_file, cc, cerr);
  if (err) return (err);

  return (err);
}
