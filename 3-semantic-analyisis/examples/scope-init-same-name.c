// RUN-WITH-ARGS: -dump-symbol-table

int foo(int x)
{
    int x = x + 2; // shadows the function parameter, and is initialised with the parameter's value + 2
    return x; // returns the local variable
}
