// RUN-WITH-ARGS: -dump-symbol-table

int foo(int arg0, int arg1)
{
    int var0;
    int var1;
    int var2;

    return does_not_exist;
}
