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