void do_print()
{
    int i = 42;
    int j = 100;
    int k;
    k = 7;

    print(i);
    print(j);
    print(k);
    k = 14;
    print(k);
}

int main()
{
    int j = 50;
    do_print();
    print(j);

    return 0;
}
