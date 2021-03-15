void do_print(int i)
{
    print(i);
}

void do_print8(int i0, int i1, int i2, int i3, int i4, int i5, int i6, int i7)
{
    print8(i0, i1, i2, i3, i4, i5, i6, i7);
}

int main()
{
    do_print(1);

    do_print8(1, 2, 3, 4, 5, 6, 7, 8);

    return 0;
}
