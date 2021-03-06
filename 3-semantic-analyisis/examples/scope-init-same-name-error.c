// RUN-WITH-ARGS: -dump-symbol-table

int foo()
{
    int x = x + 2; // should throw an error: undefined variable x
    return x;
}
