void add(int *c, int *a, int *b, unsigned int n)
{
    for (int i = 0; i < 16 * n; ++i)
    {
        c[i] = a[i] + b[i];
    }
}
