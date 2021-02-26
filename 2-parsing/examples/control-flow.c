int ifs()
{
    if (1 == 2) {
        42;
    }

    if (1 == 2)
        42;
    else
        64;
}

int whiles()
{
    while (1 == 2) {
        42;
    }

    while (1 == 2) ;
}

int returns()
{
    return;
    return 1 + 2;
}
