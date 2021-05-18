# VHDLproc

VHDLproc is a simple command line VHDL preprocessor written in Python following the conditional compilation directives outlined in VHDL-2019

## Usage

### Command Line Options
```
usage: vhdlproc.py [-h] [-i I] [-o O] [-D IDENTIFIER=value]

VHDLproc v1.2.0 - VHDL Preprocessor

optional arguments:
  -h, --help           show this help message and exit
  -i I                 Input file (Omit to read from stdin)
  -o O                 Output file (Omit to print to stdout)
  -D IDENTIFIER=value  Specify identifiers for conditional compilation, ex. DEBUG_LEVEL=2
```

You can read from stdin or a file, and print to stdout or another file.
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

`if {CONDITIONAL} then

`elsif {CONDITIONAL} then

`else

`end [if]

`warning STRING

`error STRING           -   Print STRING to standard error output stream
                            Will force close VHDLproc without saving
```

## Examples

More examples included under `tests/`.

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
