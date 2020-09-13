
def fib(x)
    fib(x - 1) + fib(x - 2)
;

def test(x)
    (1 + x + 2) * (2 + x + 1)
;

def test2(x)
    (1 + 2 + x) * (x + (1 + 2))
;

test(40);

# extern sin(arg);
# extern cos(arg);
# extern atan2(arg1, arg2);

