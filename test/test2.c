void write(int fd, long int s, int n);

void test(int a)
{
    write(1, "8", a);  // Directo con a, sin local x
    write(1, "\n", 1);
}

int main(void)
{
    test(8);  // Hardcode 8, sin binary op
    return 0;
}
