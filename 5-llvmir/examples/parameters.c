void print_numbers(int i, float f)
{
    // ints
    print(i);
    i = i * 2;
    print(i);

    // floats
    print_f(f);
    f = f * 2.0;
    print_f(f);
}

void print_strings(string s)
{
    print_s("Printing string:");

    print_s(s);
    s = "String printed.";
    print_s(s);
}

int main()
{
    print_numbers(1, 1.5);
    print_numbers(2, 2.5);

    print_strings("foo");
    print_strings("bar");

    return 0;
}
