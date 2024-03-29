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
usage: vhdlproc.py [-h] [-D IDENTIFIER=value] [-o DIRECTORY] [-e EXTENSION] [--parse-comments]
                   [--self-test] [--log-level LEVEL]
                   [input ...]

VHDLproc v2.3 - VHDL Preprocessor

positional arguments:
  input                Input files (will skip over files with the output extension)

options:
  -h, --help           show this help message and exit
  -D IDENTIFIER=value  Specify identifiers for conditional compilation, ex. DEBUG_LEVEL=2
  -o DIRECTORY         Directory to store parsed files
  -e EXTENSION         Output extension for processed files (defaults to '.proc.vhdl')
  --parse-comments     Parse commented directives as though they aren't commented, overwrite original
                       file. Disables skipping based on file extension
  --self-test          Run a self-test to ensure functionality
  --log-level LEVEL    Configure the logging level
```

A basic example, where VHDLproc will parse each input file, output the processed text to a new file with a given extension, and the processed files are then passed to GHDL:

```bash
vhdlproc *.vhdl              # preprocess all the files
ghdl -a --std=08 *.proc.vhdl # pass processed files to ghdl
ghdl -r --std=08 testbench   # run simulation
```

As VHDLproc also outputs each of the processed filenames to STDOUT, this would also work:
```bash
ghdl -a --std=08 $(vhdlproc *.vhdl)
ghdl -r --std=08 testbench
```

The parsed files can also be stored to another directory:
```bash
vhdlproc *.vhdl -o build/     # preprocess all the files and store in build/
ghdl -a --std=08 build/*.vhdl # pass processed files in build/ to ghdl
ghdl -r --std=08 testbench    # run simulation
```

Commented directives can also be parsed in-place, including replacing `include` directives:

```bash
vhdlproc *.vhdl --parse-comments # parse commented directives and overwrite original file
ghdl -a --std=08 *.vhdl       # same exact files that were passed to ghdl
ghdl -r --std=08 testbench    # run simulation
```

### Python Library

Parse files (will automatically set the include path):

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

parsed_text = processor.parse(code, identifiers=identifiers, include_path="path/to/pull/include/directives/from")

# Parse string
code = '''
`warning "Hello"
constant test_var : integer := 100
`if TOOL_VERSION < "2.0" then
`error "UNSUPPORTED VHDLPROC VERSION"
`end
`include "some/file.vhdl"
'''

parsed_text = processor.parse(code, identifiers=identifiers, include_path="path/to/pull/include/directives/from")
```

### Preprocessor Directives (what you put in your VHDL files)

```vhdl
-- VHDL-2019 directives

`if {CONDITIONAL} then

`elsif {CONDITIONAL} then

`else

`end [if]

`warning "STRING"       --   Print STRING to standard error output stream

`error "STRING"         --   Print STRING to standard error output stream
                        --   Will force close VHDLproc without saving

-- Additional extensions not part of VHDL-2019

`define LABEL "STRING"  --   Gives LABEL the value of STRING for
                        --   conditional statements

`include "FILENAME"     --   Include another file relative to
                        --   the location of the source

`end include "FILENAME" --   This is a counterpart to `include for parsing commented directives
                        --   in-place, should not be used directly (added automatically)
                        --   Sets the bound of where to replace when re-including a file
```

### Identifiers (or Labels)

By default, `TOOL_NAME` is set to `VHDLproc` and `TOOL_VERSION` is set to the current version of the code, these cannot be changed.

## Todo

- [ ] Seperate infix definitions, tests, and the main components of VHDLproc into their own files
- [ ] Prevent a file from including itself (to prevent infinite loops)
- [ ] Modify text and file operations to work on Windows (if they don't already)
- [ ] Throw an error if a `` `warning `` or `` `error `` string isn't wrapped in quotes
- [x] Parse comments / files in-place
- [x] Fix precedence of operators
- [x] Add the option to the CLI to take in a series of file inputs, process them, save the individual results to temporary files (i.e. in `/tmp/` or a local path), then return all of the filepaths. This would be useful for doing this with GHDL: `ghdl -a $(vhdlproc *.vhdl)`. 

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
-- `end include "include-to.vhdl"
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

### Parsing Comments

With the flag `--parse-comments`, directives are executed in-place as if they weren't commented. Code added by an `include` directive is replaced with an updated version, bounded by a corresponding `end include`.

Input:
```vhdl
-- `warning "== Including file =="
-- `define Include_file "TRUE"

-- `if INCLUDE_FILE = "TRUE" then
-- `include "../tests/include-to.vhdl"
component OLD_CODE is
  port(
	  a : in unsigned(3 downto 0);
	  b : in unsigned(3 downto 0);
	  s : in std_logic_vector(1 downto 0);
	  y : out unsigned(3 downto 0)
  );
end component;
-- `end include "../tests/include-to.vhdl"
-- `else
-- `error "Not including thing"
-- `end if

-- `warning "== Not including file =="
-- `define Include_file "false"
-- `define passed ""

-- `if INCLUDE_FILE = "TRUE" then
-- `include "../tests/include-to.vhdl"
-- `else
-- `end if

-- `if passed /= "" then
-- `Warning "Failed"
-- `else
-- `Warning "Passed"
-- `end
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
`if include_file = "false" then
`error "Failed"
`else
`warning "Passed"
`end

`define passed "failed"
```

Output:
```vhdl
-- `warning "== Including file =="
-- `define Include_file "TRUE"

-- `if INCLUDE_FILE = "TRUE" then
-- `include "../tests/include-to.vhdl"
component pll is
    port (
        clk_in : in std_logic;
        clk_out : out std_logic;
        clk_locked : out std_logic
    );
end component;
-- `if include_file = "false" then
-- `error "Failed"
-- `else
-- `warning "Passed"
-- `end

-- `define passed "failed"
-- `end include "../tests/include-to.vhdl"
-- `else
-- `error "Not including thing"
-- `end if

-- `warning "== Not including file =="
-- `define Include_file "false"
-- `define passed ""

-- `if INCLUDE_FILE = "TRUE" then
-- `include "../tests/include-to.vhdl"
-- `else
-- `end if

-- `if passed /= "" then
-- `Warning "Failed"
-- `else
-- `Warning "Passed"
-- `end
```
