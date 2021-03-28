int fibonacci(int n)
{
    int first = 0;
    int second = 1;

    if (n < 0)
        return -1; // invalid argument

    if (n == 0)
        return first;
    else if (n == 1)
        return second;

    for (int i = 0; i < n; i = i + 1)
    {
        int tmp = first; // remember old value of first

        first = second;
        second = tmp + second;
    }

    return first;
}

int main()
{
    int i = read();

    while (i >= 0)
    {
        int res = fibonacci(i);

        print(res);

        i = read();
    }

    return 0;
}
