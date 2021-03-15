int fibonacci(int n)
{
    int first = 0;
    int second = 1;
    int tmp;
    int i;

    if (n < 0)
        return -1; // invalid argument

    if (n == 0)
        return first;
    else if (n == 1)
        return second;

    for (i = 0; i < n; i = i + 1)
    {
        tmp = first; // remember old value of first

        first = second;
        second = tmp + second;
    }

    return first;
}

int main()
{
    int i = read();
    int res;

    while (i >= 0)
    {
        res = fibonacci(i);

        print(res);

        i = read();
    }

    return 0;
}
