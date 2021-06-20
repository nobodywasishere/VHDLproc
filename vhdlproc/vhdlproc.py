#!/usr/bin/env python

import os, sys
import shlex
import argparse
import logging

# from vhdlproc_infix import *
# import vhdlproc_test

logger = logging.getLogger(__name__)

__version__ = "2.0"


# Infix class based on https://code.activestate.com/recipes/384122/
# Originally licensed under the PSF License
class Infix:
    def __init__(self, function):
        self.function = function
    def __or__(self, other):
        return self.function(other)
    def __ror__(self, other):
        return Infix(lambda x, self=self, other=other: self.function(other, x))
    def __truediv__(self, other):
        return self.function(other)
    def __rtruediv__(self, other):
        return Infix(lambda x, self=self, other=other: self.function(other, x))
    def __call__(self, value1, value2):
        return self.function(value1, value2)

# functions for implementing VHDL operators in python for evaluating statements
# relational
r_eq   = Infix(lambda x,y:      x  == y)
r_neq  = Infix(lambda x,y:      x  != y)
r_lt   = Infix(lambda x,y:      x   < y)
r_leq  = Infix(lambda x,y:      x  <= y)
r_gt   = Infix(lambda x,y:      x   > y)
r_geq  = Infix(lambda x,y:      x  >= y)
# logical
l_and  = Infix(lambda x,y:      x and y)
l_or   = Infix(lambda x,y:      x  or y)
l_nand = Infix(lambda x,y: not (x and y))
l_nor  = Infix(lambda x,y: not (x  or y))
l_xor  = Infix(lambda x,y:      x  != y)
l_xnor = Infix(lambda x,y:      x  == y)


class VHDLproc:
    __tool_name    = "VHDLproc"
    __version      = __version__
    __directives   = ['`warning', '`error', '`if', '`elsif', '`else', '`end', '`include', '`define']
    __comment_char = "-- "

    def __eval(self, statement, identifiers):

        statement = ' '.join(statement)

        # print(f'VHDLproc: Evaluating statement {statement}')

        statement = statement.lower()

        operators = ('=', '/=', '<', '<=', '>', '>=',
                     'and', 'or', 'nand', 'nor', 'xor', 'xnor')

        for name in statement.replace('(', ' ').replace(')', ' ').split(' '):
            if name not in identifiers and name not in operators and "'" not in name and '"' not in name:
                logger.warning(f'VHDLproc: Warning: Setting empty identifier {name}')
                identifiers[name] = ""
                

        # replacing VHDL operators with Python ones to preserve VHDL 
        # operator precedence, / has higher precedence than |
        statement = statement.replace(   ' = ',  ' /r_eq/ ')
        statement = statement.replace(  ' /= ', ' /r_neq/ ')
        statement = statement.replace(   ' < ',  ' /r_lt/ ')
        statement = statement.replace(  ' <= ', ' /r_leq/ ')
        statement = statement.replace(   ' > ',  ' /r_gt/ ')
        statement = statement.replace(  ' >= ', ' /r_geq/ ')

        statement = statement.replace( ' and ', ' |l_and| ')
        statement = statement.replace(  ' or ',  ' |l_or| ')
        statement = statement.replace(' nand ',' |l_nand| ')
        statement = statement.replace( ' nor ', ' |l_nor| ')
        statement = statement.replace( ' xor ', ' |l_xor| ')
        statement = statement.replace(' xnor ',' |l_xnor| ')

        for id in identifiers:
            locals()[id] = identifiers[id]

        # print(f'Evaluating statement {statement}: {eval(statement)}')

        return eval(statement)

    def parse_file(self, file, identifiers={}):
        code = []
        for line in open(file):
            code.append(line.rstrip('\n'))

        include_path = '/'.join(file.split('/')[:-1]) + '/'
        
        return self.parse(code, identifiers, include_path=include_path)

    def parse(self, code, identifiers={}, include_path="./"):
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

        temp_ids = {}

        for id in identifiers:
            temp_ids[id.lower()] = identifiers[id].lower()

        identifiers = temp_ids

        ifstack = [[True, False]]
        line_i = -1

        while line_i < len(code) - 1:
            line_i = line_i + 1
            line = code[line_i].split('--')[0] # ignore comments

            # print(f'\nline:    {line}')

            if len(line.strip()) == 0: # skip empty lines
                continue

            if '`' == line.strip()[0]: # if this line is a directive

                # https://stackoverflow.com/a/79985
                # remove blank space and split words into a list
                # don't separate quotes
                directive = shlex.split(line.strip(), posix=False)
                code[line_i] = self.__comment_char + code[line_i]

                directive[0] = directive[0].lower()

                if ifstack[-1][0]:
                    # if it's not a supported directive
                    if directive[0] not in self.__directives:
                        raise Exception(f"VHDLproc: Line {line_i+1}: Unknown directive: {line.strip().split(' ')[0]}")

                    # print warning messages if not commented out
                    elif directive[0] == '`warning':
                        if len(directive) < 2:
                            raise Exception(f'VHDLproc: Line {line_i+1}: `warning directive requires a message')
                        warning_message = directive[1][1:-1]
                        logger.warning(f'VHDLproc: Warning: Line {line_i+1}: {warning_message}')

                    # print error messages if not commented out
                    elif directive[0] == '`error':
                        if len(directive) < 2:
                            raise Exception(f'VHDLproc: Line {line_i+1}: `error directive requires a message')
                        error_message = directive[1][1:-1]
                        logger.error(f'VHDLproc: Line {line_i+1}: Error: {error_message}')
                        exit(1)

                    # open file and append source code at this location
                    elif directive[0] == '`include':
                        filename = include_path + " ".join(directive[1:])[1:-1]
                        include = []
                        for incl_line in open(filename):
                            include.append(incl_line.rstrip('\n'))
                        code = code[:line_i+1] + include + code[line_i+1:]

                    elif directive[0] == '`define':
                        if len(directive) != 3:
                            raise Exception(f'VHDLproc: Line {line_i+1}: `error directive requires a label and value')
                        identifiers[directive[1].lower()] = directive[2].lower()[1:-1]

                if directive[0] == '`if':
                    if directive[-1] != "then":
                        raise Exception(f'VHDLproc: Line {line_i+1}: `if directive requires a then')
                    if ifstack[-1][0]:
                        resp = self.__eval(directive[1:-1], identifiers)
                        ifstack.append([resp, resp])
                    else:
                        ifstack.append([False, True])

                elif directive[0] == '`elsif':
                    if directive[-1] != "then":
                        raise Exception(f'VHDLproc: Line {line_i+1}: `elsif directive requires a then')
                    resp = self.__eval(directive[1:-1], identifiers)
                    ifstack[-1][0] = not ifstack[-1][1] and resp
                    ifstack[-1][1] = ifstack[-1][1] or resp

                elif directive[0] == '`else':
                    if ifstack[-2][0]:
                        ifstack[-1][0] = not ifstack[-1][1]

                elif directive[0] == '`end':
                    ifstack.pop()

            # don't comment out lines that are already commented out
            if not ifstack[-1][0] and code[line_i].strip()[:2] != "--":
                code[line_i] = self.__comment_char + code[line_i]

            # print(f'ifstack: {ifstack}')

        if len(ifstack) > 1:
            raise Exception(f"VHDLproc: Line {line_i+1}: Missing `end [ if ]")

        return '\n'.join(code)

def test_file(name, identifiers={}):
    print(f'\n== Testing {name} ==')
    proc = VHDLproc()
    filename = os.path.dirname(__file__) + f'/tests/{name}.vhdl'
    passed = True

    try:
        parsed = proc.parse_file(filename, identifiers=identifiers)
    except Exception as e:
        print(e)
        print("Failed")
        passed = passed and False

    if passed:
        print('== Passed ==')
    else:
        print('== Failed ==')

    return passed


def test_all():
    return not (test_file('include') and test_file('and') and test_file('nest'))

def cli():
    parser = argparse.ArgumentParser(description=f"VHDLproc v{__version__} - VHDL Preprocessor")
    parser.add_argument('-i', help='Input file (Omit to read from stdin)')
    parser.add_argument('-o', help='Output file (Omit to print to stdout)')
    parser.add_argument('--test', action='store_true', help='Tests VHDLproc to ensure functionality')
    parser.add_argument('-D', action="append", metavar="IDENTIFIER=value", help='Specify identifiers for conditional compilation, ex. DEBUG_LEVEL=2')
    args = parser.parse_args()

    if args.test:
        retn = test_all()
        exit(retn)
        

    identifiers = {}

    if args.D:
        for id in args.D:
            identifiers[id.split('=')[0]] = str(id.split('=')[1])

    proc = VHDLproc()
    if args.i:
        parsed_code = proc.parse_file(args.i, identifiers=identifiers)
    else:
        code = []
        for line in sys.stdin:
            code.append(line.rstrip('\n'))
        parsed_code = proc.parse(code, identifiers=identifiers)


    if args.o:
        open(args.o, 'w+').write(parsed_code)
    else:
        print(parsed_code)

if __name__ == "__main__":
    cli()