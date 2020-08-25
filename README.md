# VHDLproc

VHDLproc is a simple command line VHDL preprocessor with C-like preprocessor functions.

Fork of `vpp` version 2.0.3d which is Copyright (c) 2006-2020 Takashige Sugie <takashige@users.sourceforge.net> and licensed under GPL version 2 or later. `vpp` uses CVS as it's version control software, and I was not able to translate version history from before the fork. The latest version of vpp is available [here](https://sourceforge.net/projects/vhdlpp/).

## Build and Install
To build/install, cd into the `src` directory and run
```
make; make install -DINSTALL_PREFIX="/where/to/install"
```
where `INSTALL_PREFIX` is where you want VHDLproc installed; defaults to `/usr/local/bin` if not included. Uninstall using `make uninstall`.

## Usage

### Command Line Options

```
Usage: vhdlproc [-DCstvhl] InputFile OutputFile

Options:
-D, --define=<label>        Define <label>
-C, --comment=<char>        Comment out character (default:'-')
-s, --silent                Print no message
-t, --template              Create a template file
-v, --version               Print version
-h, --help                  Print vhdlproc options
-l, --listppd               Print preprocessing directives
```


### Preprocessor Directives (what you put in your VHDL files)
```
/* ... */               -   Comment out from /* to */

#include "FILENAME"     -   Include another file here

#define LABEL           -   Define LABEL for #ifdef, #ifndef and #elif

#define LABEL "STRING"  -   Replace LABEL by STRING; must be a single word

#rand LABEL FORMAT      -   Replace LABEL by generated random characters
                            according to FORMAT. FORMAT has an alphabet
                            for radix and a digit number for generating,
                            The radix character can be set by 'B'(bin),
                            'D'(dec), 'H'(hex) and 'A'(alphabet)

#undef LABEL            -   Undefine LABEL by #define and #rand

#ifdef LABEL            -   If LABEL is defined, then the following is valid
                            until #elif, #else or #endif (Nest depth < 30)

#ifndef LABEL           -   If LABEL is not defined, then following program
                            is valid until #elif, #else or #endif

#elif                   -   Equal to describing #else and #ifdef

#else                   -   Reverse condition for #ifdef, #ifndef and #elif

#endif                  -   Terminator for #ifdef, #ifndef, #elif and #else

#for LABEL              -   Duplicate program code until #endfor LABEL times
                            These cannot be nested.

#endfor                 -   Terminator for #for

#message "STRING"       -   Print STRING to the standard output stream
```

## Examples

### Multi-line comment
Input File:
```
    /*
    a <= b AND c;
    d <= e AND NOT f;
    */
    a <= b OR c;
    d <= e OR f;

    g <= a AND d;

```
Output File:
```
        --
    --    a <= b AND c;
    --    d <= e AND NOT f;
    --    */
    a <= b OR c;
    d <= e OR f;

    g <= a AND d;
```

### Include File
process.vhdl:
```
    process (clk) begin
        if (rising_edge(clk)) then
            count <= count + 1;
        end if;
    end process;
```
Input File:
```
    signal count : unsigned(3 downto 0) := 4b"0";

    begin

    #include "process.vhdl"

```
Output File:
```
    signal count : unsigned(3 downto 0) := 4b"0";

    begin

    --#include "process.vhdl"
    process (clk) begin
        if (rising_edge(clk)) then
            count <= count + 1;
        end if;
    end process
```

### Print Message
Input File:
```
    #ifdef PRINTMESSAGE
    #message "You asked me to print this message"
    #endif

```
Output File:
```
    --#ifdef PRINTMESSAGE
    --#message "You asked me to print this message"
    --#endif
```
stdout:
```
You asked me to print this message
```

## Tests

- [x] comment.vhdl - test for multiline comments
- [x] define.vhdl - test for defining a label and using it with if statements
- [x] for.vhdl - test for for loops with integer
- [x] include.vhdl / include-to.vhdl - test for including one file in another
- [x] message.vhdl - test for printing string to stdout
- [x] rand.vhdl - test for rand label generation
- [ ] undef.vhdl - test for undefining a label (currently produces segmentation fault)

## TODO
* Fix Makefile and include automatic testing
* Batch convert files
* Default conversion file name based on original filename
