void write(int fd, long int s, int n);

void test(int a)
{
    int x = a;  // Local: init con arg, store en -8(%rbp)
    write(1, "x=", 2);  // Imprime "x="
    write(1, "8", x);   // Imprime "8" (x=8, longitud x)
    write(1, "\n", 1);  // Newline
}

int main(void)
{
    test(8);
    return 0;
}
