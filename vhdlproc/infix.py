# Based on https://code.activestate.com/recipes/384122/
# Originally licensed under the PSF License

class Infix:
    def __init__(self, function):
        self.function = function
    def __or__(self, other):
        return self.function(other)
    def __ror__(self, other):
        return Infix(lambda x, self=self, other=other: self.function(other, x))
    def __truediv__(self, other):
        return self.function(other)
    def __rtruediv__(self, other):
        return Infix(lambda x, self=self, other=other: self.function(other, x))
    def __call__(self, value1, value2):
        return self.function(value1, value2)

# functions for implementing VHDL operators in python for evaluating statements

# relational
r_eq   = Infix(lambda x,y:      x  == y)
r_neq  = Infix(lambda x,y:      x  != y)
r_lt   = Infix(lambda x,y:      x   < y)
r_leq  = Infix(lambda x,y:      x  <= y)
r_gt   = Infix(lambda x,y:      x   > y)
r_geq  = Infix(lambda x,y:      x  >= y)

# logical
l_and  = Infix(lambda x,y:      x and y)
l_or   = Infix(lambda x,y:      x  or y)
l_nand = Infix(lambda x,y: not (x and y))
l_nor  = Infix(lambda x,y: not (x  or y))
l_xor  = Infix(lambda x,y:      x  != y)
l_xnor = Infix(lambda x,y:      x  == y)