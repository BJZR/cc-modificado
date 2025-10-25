void write(int fd, long int s, int n);

void test(int a)
{
    write(0, "123456789\n", a);
    write(0, "\n", 1);
}

int main()
{
    test(3 + 5);
    return 0;
}
