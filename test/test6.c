void write(int fd, long int s, int n);

void test(int a)
{
    int x = a;
    int y = x + 5;  // y=13
    write(1, "x=", 2);
    write(1, "y=", 2);
    write(1, "13", 2);  // ¡Len=2, no y!
    write(1, "\n", 1);
}

int main(void)
{
    test(8);
    return 0;
}
