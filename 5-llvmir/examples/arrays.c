int main()
{
    // Integers
    int x[3];
    x[0] = 1;
    x[1] = 2;
    x[2] = 3;

    print(x[0]);
    print(x[1]);
    print(x[2]);

    x[0] = x[0] * 2;
    x[1] = x[1] * 2;
    x[2] = x[2] * 2;

    print(x[0]);
    print(x[1]);
    print(x[2]);

    // Floats
    float y[2];
    y[0] = 1.0;
    y[1] = 2.0;

    print_f(y[0]);
    print_f(y[1]);

    float tmp = y[0];
    y[0] = y[1];
    y[1] = tmp;

    print_f(y[0]);
    print_f(y[1]);

    // Strings
    string z[3];
    z[0] = z[1] = z[2] = "Test!";
    z[1] = "foo";

    print_s(z[0]);
    print_s(z[1]);
    print_s(z[2]);

    z[0] = "bar";
    z[2] = "quux";

    print_s(z[0]);
    print_s(z[1]);
    print_s(z[2]);

    return 0;
}
