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

# functions for implementing xor and xnor operators for evaluating statements
eq   = Infix(lambda x,y: x == y)
neq  = Infix(lambda x,y: x != y)
xor  = Infix(lambda x,y: x != y)
xnor = Infix(lambda x,y: x == y)