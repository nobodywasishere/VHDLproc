/*
    vpp / VHDL PreProcessor
    Copyright (c) Takashige Sugie <takashige@users.sourceforge.net>, All rights reserved.
    Last update: <2020-02-14 23:38:53>
*/

/*
    VHDLproc - VHDL preprocessor
    Copyright (c) 2020 Michael Riegert <michael@eowyn.net>
*/

/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
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

# ifndef VERSION
# define VERSION 0
# endif

# define TRUE 1
# define FALSE 0

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "vpp.h"

#define FONT_DEFAULT      "\x1b[0m"
#define FONT_BOLD         "\x1b[1m"

void help();
void list_ppd();
void print_copyright();

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
            {"listppd", no_argument, 0, 'l'},
            {0, 0, 0, 0}
        };

        while ((option = getopt_long(argc, argv, "D:C:stvhl", LongOptions, 0)) != EOF) {
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
                    printf("vhdlproc version %s\n", VERSION);
                    return (0);
                case 'l':
                    list_ppd();
                case 'h':
                default:
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

void help() {
    print_copyright();
    printf(
        "------------------------------------------------------------------------------\n"
        "Usage: vhdlproc [-DCstvhl] InputFile OutputFile\n\n"

        "Options:\n"
        "-D, --define=<label>        Define <label>\n"
        "-C, --comment=<char>        Comment out character (default:'-')\n"
        "-s, --silent                Print no message\n"
        "-t, --template              Create a template file\n"
        "-v, --version               Print version\n"
        "-h, --help                  Print vhdlproc options\n"
        "-l, --listppd               Print preprocessing directives\n"
        "------------------------------------------------------------------------------\n"
    );

    exit(0);
}

void list_ppd() {
    // Lists the preprocessor directives
    print_copyright();
    printf(
        "------------------------------------------------------------------------------\n"
        "Preprocessor directives      Effect\n"
        "------------------------------------------------------------------------------\n"
        "/* ... */                    Comment out from /* to */\n\n"
        "#include \"" FONT_BOLD "FILENAME" FONT_DEFAULT "\"          Include another file here\n\n"
        "#define " FONT_BOLD "LABEL" FONT_DEFAULT "                Define " FONT_BOLD "LABEL" FONT_DEFAULT " for #ifdef, #ifndef and #elif\n\n"
        "#define " FONT_BOLD "LABEL" FONT_DEFAULT " \"" FONT_BOLD "STRING" FONT_DEFAULT "\"       "
        "Replace " FONT_BOLD "LABEL" FONT_DEFAULT " by " FONT_BOLD "STRING" FONT_DEFAULT "; must be a single word\n\n"
        "#rand " FONT_BOLD "LABEL FORMAT" FONT_DEFAULT"           Replace " FONT_BOLD "LABEL" FONT_DEFAULT " by generated random characters\n"
        "                             according to " FONT_BOLD "FORMAT" FONT_DEFAULT ". FORMAT has an alphabet \n"
        "                             for radix and a digit number for generating,\n"
        "                             The radix character can be set by 'B'(bin),\n"
        "                             'D'(dec), 'H'(hex) and 'A'(alphabet)\n\n"
        "#undef " FONT_BOLD "LABEL" FONT_DEFAULT "                 Undefine " FONT_BOLD "LABEL" FONT_DEFAULT " by #define and #rand\n\n"
        "#ifdef " FONT_BOLD "LABEL" FONT_DEFAULT "                 If " FONT_BOLD "LABEL" FONT_DEFAULT " is defined, then the following is valid\n"
        "                             until #elif, #else or #endif (Nest depth < 30)\n\n"
        "#ifndef " FONT_BOLD "LABEL" FONT_DEFAULT "                If " FONT_BOLD "LABEL" FONT_DEFAULT " is not defined, then following program\n"
        "                             is valid until #elif, #else or #endif\n\n"
        "#elif                        Equal to describing #else and #ifdef\n\n"
        "#else                        Reverse condition for #ifdef, #ifndef and #elif\n\n"
        "#endif                       Terminator for #ifdef, #ifndef, #elif and #else\n\n"
        "#for " FONT_BOLD "LABEL" FONT_DEFAULT "                   Duplicate program code until #endfor " FONT_BOLD "LABEL" FONT_DEFAULT " times\n"
        "                             These cannot be nested.\n\n"
        "#endfor                      Terminator for #for\n\n"
        "#message \"" FONT_BOLD "STRING" FONT_DEFAULT "\"            Print " FONT_BOLD "STRING" FONT_DEFAULT " to the standard output stream\n"
    );

    exit(0);
}

void print_copyright() {
    printf(
        "------------------------------------------------------------------------------\n"
        "VHDLproc / VHDL PreProcessor, version %s\n"
        "Copyright (c) 2020 Michael Riegert <michael@eowyn.net>\n"
        "Copyright (c) 2006-2020 Takashige Sugie <takashige@users.sourceforge.net>\n"
        "Licensed under General Public License version 3 or later\n\n"

        "Fork of vpp / VHDL PreProcessor, version 2.0.3d\n"
        "https://sourceforge.net/projects/vhdlpp/\n"
        "Copyright (c) 2006-2020 Takashige Sugie <takashige@users.sourceforge.net>\n"
        "Licensed under General Public License version 2 or later\n"
        "------------------------------------------------------------------------------\n\n",
        VERSION
    );
}
