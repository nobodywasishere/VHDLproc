/*
  vpp / VHDL PreProcessor
  Copyright (c) Takashige Sugie <takashige@users.sourceforge.net>, All rights reserved.
  Last update: <2020-02-14 23:38:53>
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
#include <getopt.h>
#include <string.h>
#include "vpp.h"

#define FONT_DEFAULT      "\x1b[0m"
#define FONT_BOLD         "\x1b[1m"

void help()
{
  printf(
	 "Usage: vpp [-DCstvh] OriginalFile ConvertedFile\n"
	 "Options:\n"
	 "-D, --define=<label>\t\t Define <label>\n"
	 "-C, --comment=<char>\t\t Comment out symbol character (default:'-')\n"
	 "-s, --silent\t\t\t Print no message\n"
	 "-t, --template\t\t\t Create a template file\n"
	 "-v, --version\t\t\t Print version\n"
	 "-h, --help\t\t\t Print help\n"
	 "------------------------------------------------------------------------------\n"
	 "Preprocessor directives\t\t Effect\n"
	 "------------------------------------------------------------------------------\n"
	 "/* ... */\t\t\t Comment out from /* to */\n"
	 "#include \"" FONT_BOLD "FILENAME" FONT_DEFAULT "\"\t\t Include another file whose file name is " FONT_BOLD "FILENAME" FONT_DEFAULT " here\n"
	 "#define " FONT_BOLD "LABEL" FONT_DEFAULT "\t\t\t Define " FONT_BOLD "LABEL" FONT_DEFAULT " for #ifdef, #ifndef and #elif\n"
	 "#define " FONT_BOLD "LABEL" FONT_DEFAULT " \"" FONT_BOLD "STRING" FONT_DEFAULT "\"\t\t Replace " FONT_BOLD "LABEL" FONT_DEFAULT " which must be a single word, by " FONT_BOLD "STRING" FONT_DEFAULT "\n"
	 "#rand " FONT_BOLD "LABEL FORMAT" FONT_DEFAULT"\t\t Replace " FONT_BOLD "LABEL" FONT_DEFAULT " by generated random characters according to " FONT_BOLD "FORMAT" FONT_DEFAULT "\n"
	 "                       \t\t FORMAT has an alphabet for radix and a digit number to generating\n"
	 "                       \t\t The radix character can be set by 'B'(bin), 'D'(dec), 'H'(hex) and 'A'(alphabet)\n"
	 "#undef " FONT_BOLD "LABEL" FONT_DEFAULT "\t\t\t Undefine " FONT_BOLD "LABEL" FONT_DEFAULT " by #define and #rand\n"
	 "#ifdef " FONT_BOLD "LABEL" FONT_DEFAULT "\t\t\t If " FONT_BOLD "LABEL" FONT_DEFAULT " is defined, then following program is valid until #elif, #else or #endif (Depth of nest is less than 30)\n"
	 "#ifndef " FONT_BOLD "LABEL" FONT_DEFAULT "\t\t\t If " FONT_BOLD "LABEL" FONT_DEFAULT " is not defined, then following program is valid until #elif, #else or #endif\n"
	 "#elif\t\t\t\t Equal to describing #else and #ifdef\n"
	 "#else\t\t\t\t Reverse condition for #ifdef, #ifndef and #elif\n"
	 "#endif\t\t\t\t Terminator for #ifdef, #ifndef, #elif and #else\n"
	 "#for " FONT_BOLD "LABEL" FONT_DEFAULT "\t\t\t Duplicate program code until #endfor " FONT_BOLD "LABEL" FONT_DEFAULT " times (Can't nest)\n"
	 "#endfor\t\t\t\t Terminator for #for\n"
	 "#message \"" FONT_BOLD "STRING" FONT_DEFAULT "\"\t\t Print " FONT_BOLD "STRING" FONT_DEFAULT " to the standard output stream\n"
	 );

  exit(0);
}

int main(int argc, char **argv)
{
  const char *orig, *conv;
  unsigned flags = 0;
  FILE *fp = stderr;
  tlist_t *d = NULL;
  char cc = '-';
  int err = 0;
  char *suffix;

  {
    extern int optind;
    int option;
    extern char *optarg;
    struct option LongOptions[] = {
      {"define", required_argument, 0, 'D'},
      {"comment", required_argument, 0, 'C'},
      {"silent", no_argument, 0, 's'},
      {"testbench", no_argument, 0, 't'},
      {"version", no_argument, 0, 'v'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };

    while ((option = getopt_long(argc, argv, "D:C:stvh", LongOptions, 0)) != EOF) {
      switch (option) {
      case 'D':
	if (optarg) {
	  d = taka_tvpp_define_label(d, optarg);
	} else {
	  help();
	}
	break;
      case 'C':
	if (optarg) {
	  cc = optarg[0];
	} else {
	  help();
	}
	break;
      case 's':
	fp = NULL;
	break;
      case 't':
	flags |= VPP_BEHAVIOR_TEMPLATE;
	break;
      case 'v':
	printf("vpp version %s\n", VERSION);
	return (0);
      case 'h':
      default:
	printf("vpp / VHDL PreProcessor, version %s\nCopyright (c) 2006-2020 Takashige Sugie <%s>, All rights reserved.\n", VERSION, PACKAGE_BUGREPORT);
	help();
      }
    }

    if (flags & VPP_BEHAVIOR_TEMPLATE) {
      if (argc != optind + 1) help();
    } else {
      if (argc != optind + 2) help();
    }
  }

  orig = argv[optind++];
  conv = argv[optind++];

  if ((suffix = strrchr(orig, '.'))) {
    if (strcmp(suffix, ".v")) {
      if (flags & VPP_BEHAVIOR_TEMPLATE) {
	err = taka_tvpp_create_template(orig, cc, fp);
      } else {
	err = taka_tvpp_convert(orig, conv, d, cc, fp);
      }
    }
  }

  return (err);
}
