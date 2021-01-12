# VHDLproc

VHDLproc is a simple command line VHDL preprocessor with C-like preprocessor functions, written in Python

Recreation of `vpp` version 2.0.3d which is Copyright (c) 2006-2020 Takashige Sugie <takashige@users.sourceforge.net> and licensed under GPL version 2 or later. The latest version of vpp is available [here](https://sourceforge.net/projects/vhdlpp/).

## Build and Install

From Linux, simply make `src/vhdlproc` executable by running `chmod +x src/vhdlproc`. Then the program can be run directly via `./src/vhdlproc`.

## Usage

### Command Line Options
```
usage: vhdlproc [-h] [-D LABEL=VALUE] [-l] [-q] [-v] [--version] [input] [output]

VHDLproc v1.1.0 - VHDL Preprocessor

positional arguments:
  input                 Input file
  output                Output file (defaults to [filename]-out.vhdl), pass - to print to stdout

optional arguments:
  -h, --help            show this help message and exit
  -D LABEL=VALUE, --define LABEL=VALUE
                        Define a label as a given value
  -l, --listppd         Print preprocessing directives
  -q, --quiet           Dont save or print output
  -v                    Verbose output
  --version             show program's version number and exit
```


### Preprocessor Directives (what you put in your VHDL files)
```
/* ... */               -   Comment out from /* to */

`include FILENAME       -   Include another file here

`define LABEL           -   Define LABEL for `ifdef and `ifndef

`define LABEL STRING    -   Replace LABEL by STRING; must be a single word

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

`else                   -   Reverse condition for `ifdef and `ifndef (when not
                            overridden by nested if)

`endif                  -   Terminator for `ifdef, `ifndef and `else

`for LABEL              -   Duplicate program code until `endfor LABEL times
                            Can be nested

`endfor                 -   Terminator for `for

`message STRING         -   Print STRING to the standard output stream
```

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
