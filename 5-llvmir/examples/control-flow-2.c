int is_even(int n)
{
    if (n % 2 == 0)
        return 1;
    else
        return 0;
}

int main()
{
    int i = read();

    while (i >= 0)
    {
        int res = is_even(i);

        if (res)
            print_s("even");
        else
            print_s("odd");

        i = read();
    }

    return 0;
}
