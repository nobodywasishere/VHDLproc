`define VHDL_VERSION "2019"
`if VHDL_VERSION >= "2008" then
constant enable_features : bool := true
`else
`warning "Certain features disabled!"
`end
