int dangling_else()
{
    if (1)
        if (2)
            a;
        else
            b;
}
