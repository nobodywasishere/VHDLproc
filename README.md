# VHDLproc

VHDLproc is a simple command line VHDL preprocessor written in Python following the conditional compilation directives outlined in VHDL-2019, with a few extensions.

## Installation

VHDLproc can be installed via pip:
```
$ pip install vhdlproc
$ vhdlproc --help
```

It can also be installed from source
```
$ git clone https://github.com/nobodywasishere/vhdlproc
$ cd vhdlproc
$ python setup.py install --user
$ vhdlproc --help
```

It can also simply be run as a standalone file
```
$ git clone https://github.com/nobodywasishere/vhdlproc
$ cd vhdlproc
$ ./vhdlproc/vhdlproc.py --help
```

## Usage

### Command Line

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
$ cat tests/define.vhdl | python vhdlproc/vhdlproc.py
`if VHDL_VERSION >= "2008" then
...
`end if
...
```

### Python Library

Parse files:

```python
from vhdlproc import VHDLproc

processor = VHDLproc()

identifiers = {"VHDL_VERSION": "2008"}

parsed_text = processor.parse_file("path/to/file.vhdl", identifiers=identifiers)
```

Parse code directly:

```python
from vhdlproc import VHDLproc

processor = VHDLproc()

identifiers = {"VHDL_VERSION": "2008"}

# Parse list of lines of text
code = [
    '`warning "Hello"',
    'constant test_var : integer := 100',
    '`if TOOL_VERSION < "2.0" then',
    '`error "UNSUPPORTED VHDLPROC VERSION"',
    '`end',
    '`include "some/file.vhdl"',
]

parsed_text_list = processor.parse_file(code, identifiers=identifiers, include_path="path/to/pull/include/directives/from")

# Parse string
code = '''
`warning "Hello"
constant test_var : integer := 100
`if TOOL_VERSION < "2.0" then
`error "UNSUPPORTED VHDLPROC VERSION"
`end
`include "some/file.vhdl"
'''

parsed_text_str = processor.parse_file(code, identifiers=identifiers, include_path="path/to/pull/include/directives/from")
```

### Preprocessor Directives (what you put in your VHDL files)

```vhdl
-- VHDL-2019 directives

`if {CONDITIONAL} then

`elsif {CONDITIONAL} then

`else

`end [if]

`warning "STRING"       -   Print STRING to standard error output stream

`error "STRING"         -   Print STRING to standard error output stream
                            Will force close VHDLproc without saving

-- Additional extensions not part of VHDL-2019

`define LABEL "STRING"  -   Gives LABEL the value of STRING for
                            conditional statements

`include "FILENAME"     -   Include another file relative to
                            the location of the source
```

### Identifiers (or Labels)

By default, `TOOL_NAME` is set to `VHDLproc` and `TOOL_VERSION` is set to the current version of the code, these cannot be changed.

## Todo

- [ ] Seperate infix definitions, tests, and the main components of VHDLproc into their own files
- [ ] Prevent a file from including itself (to prevent infinite loops)
- [ ] Modify text and file operations to work on Windows (if they don't already)
- [ ] Throw an error if a `` `warning `` or `` `error `` string isn't wrapped in quotes
- [ ] Add the option to the CLI to take in a series of file inputs, process them, save the individual results to temporary files (i.e. in `/tmp/` or a local path), then return all of the filepaths. This would be useful for doing this with GHDL: `ghdl -a $(vhdlproc -f *.vhdl)`. 

## Examples

More examples included under `vhdlproc/tests/`.

### Include File

Input:

```vhdl
`include "include-to.vhdl"
```

include-to.vhdl:

```vhdl
component pll is
    port (
        clk_in : in std_logic;
        clk_out : out std_logic;
        clk_locked : out std_logic
    );
end component;
```

Output:

```vhdl
-- `include "include-to.vhdl"
component pll is
    port (
        clk_in : in std_logic;
        clk_out : out std_logic;
        clk_locked : out std_logic
    );
end component;
```

### Define, Repeated If/Elsif

Input:
```vhdl
`define a "a"
`define b "z"

`if a = "a" and b = "b" then
a = "a" and b = "b"
`elsif a /= "a" and b = "b" then
a /= "a" and b = "b"
`elsif a = "a" and b /= "b" then
a = "a" and b /= "b"
`elsif a /= "a" and b /= "b" then
a /= "a" and b /= "b"
`else
`warning "Not supposed to be here"
`end
```

Output:
```vhdl
-- `define a "a"
-- `define b "z"

-- `if a = "a" and b = "b" then
-- a = "a" and b = "b"
-- `elsif a /= "a" and b = "b" then
-- a /= "a" and b = "b"
-- `elsif a = "a" and b /= "b" then
a = "a" and b /= "b"
-- `elsif a /= "a" and b /= "b" then
-- a /= "a" and b /= "b"
-- `else
-- `warning "Not supposed to be here"
-- `end
```

### Nested If

Input:
```vhdl
`define a "a"
`define b "b"

`if a = "a" then
`if b = "b" then
a = "a" and b = "b"
`else
a = "a" and b /= "b"
`end
`end
```

Output:
```vhdl
-- `define a "a"
-- `define b "b"

-- `if a = "a" then
-- `if b = "b" then
a = "a" and b = "b"
-- `else
-- a = "a" and b /= "b"
-- `end
-- `end
```

### VHDL Version

Input:
```vhdl
`define VHDL_VERSION "2019"
`if VHDL_VERSION >= "2008" then
constant enable_features : bool := true
`else
`warning "Certain features disabled!"
constant enable_features : bool := false
`end
```

Output:
```vhdl
-- `define VHDL_VERSION "2019"
-- `if VHDL_VERSION >= "2008" then
constant enable_features : bool := true
-- `else
-- `warning "Certain features disabled!"
-- constant enable_features : bool := false
-- `end
```