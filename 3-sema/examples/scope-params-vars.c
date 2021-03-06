// RUN-WITH-ARGS: -dump-symbol-table

int sum(int x, int y)
{
    int res = x + y;

    int x;
    int y[2];

    return x + y[0];
}

int difference(int x, int z)
{
    int res = x - z;

    int x;
    int y[2];

    return x + y[0] * z;
}
