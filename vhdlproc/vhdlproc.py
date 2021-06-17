#!/usr/bin/env python

import os, sys
import argparse
import logging

logger = logging.getLogger(__name__)

__version__ = "1.2.0"

class VHDLproc:
    __tool_name    = "VHDLproc"
    __version      = __version__
    __directives   = ['`warning', '`error', '`if', '`elsif', '`else', '`end', '`include']
    __comment_char = "-- "

    def __eval(self, statement, identifiers):
        statement = ' '.join(statement)
        statement = statement.replace(' = ',' == ')
        statement = statement.replace(' /= ',' != ')
        statement = statement.lower()

        for id in identifiers:
            locals()[id.lower()] = identifiers[id].lower()

        return eval(statement)

    def parse(self, code, identifiers={}):
        identifiers['TOOL_NAME']    = self.__tool_name
        identifiers['TOOL_VERSION'] = self.__version
        if 'VHDL_VERSION' not in identifiers:
            identifiers['VHDL_VERSION'] = ""
        if 'TOOL_TYPE' not in identifiers:
            identifiers['TOOL_TYPE']    = ""
        if 'TOOL_VENDOR' not in identifiers:
            identifiers['TOOL_VENDOR']  = ""
        if 'TOOL_EDITION' not in identifiers:
            identifiers['TOOL_EDITION'] = ""

        ifstack = [False]
        line_i = -1

        while line_i < len(code) - 1:
            line_i = line_i + 1
            line = code[line_i].split('--')[0] # ignore comments

            if len(line.strip()) == 0: # skip empty lines
                continue

            if '`' == line.strip()[0]: # if this line is a directive
                directive = line.strip().split(' ') # remove blank space and split words into a list
                code[line_i] = self.__comment_char + code[line_i]

                # if it's not a supported directive
                if directive[0] not in self.__directives:
                    raise Exception(f"VHDLproc: Line {line_i+1}: Unknown directive: {line.strip().split(' ')[0]}")

                # print warning messages if not commented out
                elif directive[0] == '`warning' and not ifstack[-1]:
                    if len(directive) < 2:
                        raise Exception(f'VHDLproc: Line {line_i+1}: `warning directive requires a message')
                    warning_message = " ".join(directive[1:]).replace('"','').replace("'","")
                    logger.warning(f'VHDLproc: Line {line_i+1}: Warning: {warning_message}')

                # print error messages if not commented out
                elif directive[0] == '`error' and not ifstack[-1]:
                    if len(directive) < 2:
                        raise Exception(f'VHDLproc: Line {line_i+1}: `error directive requires a message')
                    error_message = " ".join(directive[1:]).replace('"','').replace("'","")
                    logger.error(f'VHDLproc: Line {line_i+1}: Error: {error_message}')
                    exit(1)

                elif directive[0] == '`if':
                    if directive[-1] != "then":
                        raise Exception(f'VHDLproc: Line {line_i+1}: `if directive requires a then')
                    ifstack.append(not self.__eval(directive[1:-1], identifiers))

                elif directive[0] == '`elsif':
                    if not ifstack[-2] and self.__eval(directive[1:-1], identifiers):
                        ifstack[-1] = not ifstack[-1]

                elif directive[0] == '`else':
                    if not ifstack[-2]:
                        ifstack[-1] = not ifstack[-1]

                elif directive[0] == '`end':
                    ifstack.pop()

                # open file and append source code at this location
                # @todo Source file include location
                # @body Include from the source files local path instead of a path from where the program is executed 
                elif directive[0] == '`include':
                    filename = " ".join(directive[1:]).replace('"','').replace("'","")
                    include = []
                    for incl_line in open(filename):
                        include.append(incl_line.rstrip('\n'))
                    code = code[:line_i+1] + include + code[line_i+1:]
                    pass

            # don't comment out lines that are already commented out
            if ifstack[-1] and code[line_i].strip()[:2] != "--":
                code[line_i] = self.__comment_char + code[line_i]

        if len(ifstack) > 1:
            raise Exception(f"VHDLproc: Line {line_i+1}: Missing `end [ if ]")

        return '\n'.join(code)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=f"VHDLproc v{__version__} - VHDL Preprocessor")
    parser.add_argument('-i', help='Input file (Omit to read from stdin)')
    parser.add_argument('-o', help='Output file (Omit to print to stdout)')
    parser.add_argument('-D', action="append", metavar="IDENTIFIER=value", help='Specify identifiers for conditional compilation, ex. DEBUG_LEVEL=2')
    args = parser.parse_args()

    identifiers = {}

    if args.D:
        for id in args.D:
            identifiers[id.split('=')[0]] = str(id.split('=')[1])

    code = []
    if args.i:
        for line in open(args.i):
            code.append(line.rstrip('\n'))
    else:
        for line in sys.stdin:
            code.append(line.rstrip('\n'))

    proc = VHDLproc()
    parsed_code = proc.parse(code, identifiers=identifiers)

    if args.o:
        open(args.o, 'w+').write(parsed_code)
    else:
        print(parsed_code)
