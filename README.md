# VHDLproc

VHDLproc is a simple command line VHDL preprocessor with C-like preprocessor functions, written in Python

Recreation of `vpp` version 2.0.3d which is Copyright (c) 2006-2020 Takashige Sugie <takashige@users.sourceforge.net> and licensed under GPL version 2 or later. The latest version of vpp is available [here](https://sourceforge.net/projects/vhdlpp/).

## Build and Install

From Linux, simply make `src/vhdlproc` executable by running `chmod +x src/vhdlproc`. Then the program can be run directly via `./src/vhdlproc`. Requires the Python libs `os`, `sys`, `random`, `datetime`, and `argparse`.

## Usage

### Command Line Options
```
usage: vhdlproc [-h] [-D LABEL=VALUE] [-l] [-q] [-v] [--version] [--comment CHAR] [--directive CHAR]
                [input] [output]

VHDLproc v1.1.1 - VHDL Preprocessor

positional arguments:
  input                 Input file (Pass - to read from stdin)
  output                Output file (defaults to [filename]-out.vhdl) (pass - to print to stdout)

optional arguments:
  -h, --help            show this help message and exit
  -D LABEL=VALUE, --define LABEL=VALUE
                        Define a label as a given value
  -l, --listppd         Print preprocessing directives
  -q                    Quiet output
  -v                    Verbose output
  --version             show program's version number and exit
  --comment CHAR        Character to comment out with (default: -- )
  --directive CHAR      Character for preprocessor directives (default: ` )
```

You can read from stdin, but it can only print to stdout. This can be then used to pipe to other programs.
```
$ cat tests/define.vhdl | ./src/vhdlproc -
-- `define TEST "hello"
-- `define HELLO fun

"hello"
...
```

### Preprocessor Directives (what you put in your VHDL files)
```
/* ... */               -   Comment out from /* to */

`include FILENAME       -   Include another file relative to
                            the location of the source

`define LABEL           -   Define LABEL for `ifdef and `ifndef

`define LABEL STRING    -   Replace LABEL by STRING, can be multiple words
                            Will replace ignoring single quotes and attributes ('', ')
                            Will not replace within double quotes ("")

`rand LABEL FORMAT      -   Replace LABEL by generated random characters
                            according to FORMAT. FORMAT has an alphabet
                            for radix and a digit number for generating,
                            The radix character can be set by 'B'(bin),
                            'D'(dec), 'H'(hex) and 'A'(alphabet)

`undef LABEL            -   Undefine LABEL by `define and `rand

`ifdef LABEL            -   If LABEL is defined, then the following is valid
                            until `else or `endif
                            Can be nested

`ifndef LABEL           -   If LABEL is not defined, then following program
                            is valid until `else or `endif
                            Can be nested

`else                   -   Reverse condition for `ifdef and `ifndef

`endif                  -   Terminator for `ifdef, `ifndef and `else

`for INT                -   Duplicate program code until `endfor INT times
                            Can be nested

`endfor                 -   Terminator for `for

`message STRING         -   Print STRING to the standard output stream

`error STRING           -   Print STRING to standard error output stream
                            Will force close VHDLproc without saving
```

The preprocessor character (default: \` ) can either be changed by the command line option `--directive CHAR` or by the environment variable `VHDLPROC_DIRECTIVE`. The command line option supersedes the environment variable, which supersedes the default.

It's possible to use `#` as the preprocessor directive by passing in `--directive "#"` or by setting the environment variable `export VHDLPROC_DIRECTIVE="#"`.

The comment character (default: -- ) can either be changed by the command line option `--comment CHAR` or by the environment variable `VHDLPROC_COMMENT`. The command line option supersedes the environment variable, which supersedes the default.

There are some predefined labels as follows:
- `__FILE__`: Full path to the input file, set to `"STDIN"` when reading from stdin
- `__LINE__`: The current input line number as a string, not affected by for repeats
- `__DATE__`: The current date formatted as a string as `"Jan 01 1970"`
- `__TIME__`: The current time formatted as a string as `"00:00:00"`, 24 hour clock

## Examples

More examples included under `tests/`.

### Multiline Comment

Input:
```
/*
component pll is
    port (
        clk_in : in std_logic;
        clk_out : out std_logic;
        clk_locked : out std_logic
    );
end component;
*/

```

Output:
```
-- /*
-- component pll is
--     port (
--         clk_in : in std_logic;
--         clk_out : out std_logic;
--         clk_locked : out std_logic
--     );
-- end component;
-- */
```

### Include File

Input:
```
`include "include-to.vhdl"
```

include-to.vhdl:
```
component pll is
    port (
        clk_in : in std_logic;
        clk_out : out std_logic;
        clk_locked : out std_logic
    );
end component;
```

Output:
```
-- `include "include-to.vhdl"
component pll is
    port (
        clk_in : in std_logic;
        clk_out : out std_logic;
        clk_locked : out std_logic
    );
end component;
```

### Print Message

Input:
```
`message This test was successful
```

Command-line:
```
$ ./src/vhdlproc tests/message.vhdl
Message: This test was successful
$
```

Output:
```
-- `message This test was successful
```

### For loop

For loops work by replacing the definitions of words and resolving nested if and for loops within it, then duplicating the selection multiple times.

Input:
```
`for 2

FORK spoon

`define FORK spoon

FORK spoon

`define spoon FORK

FORK spoon

`endfor
```

Output:
```
-- `for 2

FORK spoon

-- `define FORK spoon

spoon spoon

-- `define spoon FORK

FORK FORK

--

FORK spoon

-- `define FORK spoon

spoon spoon

-- `define spoon FORK

FORK FORK

-- `endfor
```

### Define

Input:
```
`define TEST "hello"
`define HELLO fun

TEST

`define TEST2

`ifdef TEST2

`ifndef TEST4

TEST TEST TEST'test

`else

tsktstk

`endif

TEST

`else

(TEST TEST: no TEST)

`endif

`ifndef TEST3

TEST TEST TEST_TEST HELLO

`endif

`define meow_1 "hello there my name is al"

meow_1
```

output:
```
-- `define TEST "hello"
-- `define HELLO fun

"hello"

-- `define TEST2

-- `ifdef TEST2

-- `ifndef TEST4

"hello" "hello" "hello"'test

-- `else

-- tsktstk

-- `endif

"hello"

-- `else

-- (TEST TEST: no TEST)

-- `endif

-- `ifndef TEST3

"hello" "hello" TEST_TEST fun

-- `endif

-- `define meow_1 "hello there my name is al"

"hello there my name is al"
```

### Random definition

Input:
```
`rand TEST1 B10

TEST1

`rand TEST2 D5

TEST2

`rand TEST3 H20

TEST3

`rand TEST4 A50

TEST4
```

Output:
```
-- `rand TEST1 B10

1010011100

-- `rand TEST2 D5

78352

-- `rand TEST3 H20

7FAEECA221BFF5B9D27F

-- `rand TEST4 A50

XWTBBLNTTXUFLGLWUSYMFLRBNVHQKPCBTBOSKJHNQVEXOCGKDV
```
