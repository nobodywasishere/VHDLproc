`warning '== a = "a" and b = "b" =='

`define a "a"
`define b "b"

`if a = "a" and b = "b" then
`warning "Passed"
`elsif a /= "a" and b = "b" then
`error "Failed"
`elsif a = "a" and b /= "b" then
`error "Failed"
`elsif a /= "a" and b /= "b" then
`error "Failed"
`else
`error "Failed"
`end

`warning '== a /= "a" and b = "b" =='

`define a "z"
`define b "b"

`if a = "a" and b = "b" then
`error "Failed"
`elsif a /= "a" and b = "b" then
`warning "Passed"
`elsif a = "a" and b /= "b" then
`error "Failed"
`elsif a /= "a" and b /= "b" then
`error "Failed"
`else
`error "Failed"
`end

`warning '== a = "a" and b /= "b" =='

`define a "a"
`define b "z"

`if a = "a" and b = "b" then
`error "Failed"
`elsif a /= "a" and b = "b" then
`error "Failed"
`elsif a = "a" and b /= "b" then
`warning "Passed"
`elsif a /= "a" and b /= "b" then
`error "Failed"
`else
`error "Failed"
`end

`warning '== a /= "a" and b /= "b" =='

`define a "z"
`define b "z"

`if a = "a" and b = "b" then
`error "Failed"
`elsif a /= "a" and b = "b" then
`error "Failed"
`elsif a = "a" and b /= "b" then
`error "Failed"
`elsif a /= "a" and b /= "b" then
`warning "Passed"
`else
`error "Failed"
`end