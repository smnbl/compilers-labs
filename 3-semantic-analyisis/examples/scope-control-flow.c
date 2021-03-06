// RUN-WITH-ARGS: -dump-symbol-table

void ifstmts(int x)
{
    if (x == 0) {
        int x = 10 * x;

        x;
    } else {
        int x = 20 * x;

        x;
    }

    if (x == 0)
        int x = 10 * x;
    else
        int x = 20 * x;

    x;
}

void whilestmts(int x)
{
    while (x == 0) {
        int x = 10 * x;
        x;
    }

    while (x == 0)
        int x = 10 * x;

    x;
}

void forstmts(int x)
{
    for (int x = 10 * x; x < 100; x = x + 1) {
        int x = 20 * x;
        x;
    }

    for (int x = 10 * x; x < 100; x = x + 1)
        int x = 20 * x;

    x;
}
