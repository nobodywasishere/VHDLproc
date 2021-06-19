`warning '== a = "a" and b = "b" =='

`define a "a"
`define b "b"

`if a = "a" then
`if b = "b" then
`warning "Passed"
`else
`error "Failed"
`end
`else
`if b = "b" then
`error "Failed"
`else
`error "Failed"
`end
`end

`warning '== a /= "a" and b = "b" =='

`define a "z"
`define b "b"

`if a = "a" then
`if b = "b" then
`error "Failed"
`else
`error "Failed"
`end
`else
`if b = "b" then
`warning "Passed"
`else
`error "Failed"
`end
`end

`warning '== a = "a" and b /= "b" =='

`define a "a"
`define b "z"

`if a = "a" then
`if b = "b" then
`error "Failed"
`else
`warning "Passed"
`end
`else
`if b = "b" then
`error "Failed"
`else
`error "Failed"
`end
`end

`warning '== a /= "a" and b /= "b" =='

`define a "z"
`define b "z"

`if a = "a" then
`if b = "b" then
`error "Failed"
`else
`error "Failed"
`end
`else
`if b = "b" then
`error "Failed"
`else
`warning "Passed"
`end
`end