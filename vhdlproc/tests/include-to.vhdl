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