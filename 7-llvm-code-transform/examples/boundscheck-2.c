int get_index()
{
    return 20;
}

int loop_cond(int i)
{
    return i <= 0;
}

int main()
{
    int foo[10];

    int a;
    int b;
    int n;

    a = get_index();
    b = 0;

    for (int i = 0; loop_cond(i); i = i + 1)
    {
        b = b + 1;
    }

    n = a + b;

    foo[n] = 0;

    return 0;
}
