int main()
{
    // Integers
    int x;
    x = 1;
    print(x);
    x = 2;
    print(x);

    // Floats
    float y = 1.5;
    print_f(y);
    y = 2.5;
    print_f(y);

    // Strings
    string z = "Hello";
    print_s(z);
    z = "World!";
    print_s(z);

    // Multiple assignment
    int a;
    int b;
    int c;

    a = b = c = 100;

    print(a);
    print(b);
    print(c);

    return 0;
}
