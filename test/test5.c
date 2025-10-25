void write(int fd, long int s, int n);

void test(int a)
{
    int x = a;      // Local simple
    int y = x + 5;  // Binary: y = 8 + 5 = 13
    write(1, "x=", 2);    // "x="
    write(1, "y=", 2);    // "y="
    write(1, "13", y);    // "13" con n=13 (imprime "13")
    write(1, "\n", 1);    // Newline
}

int main(void)
{
    test(8);
    return 0;
}
