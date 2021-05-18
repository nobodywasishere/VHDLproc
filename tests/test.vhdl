`if VHDL_VERSION = "2008" then
`warning "Hello there"
constant hello : integer := 2008
`else
constant hello : integer := 2001
`end if

`if TOOL_NAME /= "VHDLproc" then
`error "Unsupported tool"
`end if
