`warning "== Including file =="
`define Include_file "true"

`if INCLUDE_FILE = "TRUE" then
`include "../tests/include-to.vhdl"
`else
`Warning "Not including thing"
`end if

`warning "== Not including file =="
`define Include_file "false"
`define passed ""

`if INCLUDE_FILE = "TRUE" then
`include "../tests/include-to.vhdl"
`else
`end if

`if passed /= "" then
`Warning "Failed"
`else
`Warning "Passed"
`end