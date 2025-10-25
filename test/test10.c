void write(int fd, long int s, int n);

void print_digit(int d) {
    char digit = '0' + d;
    write(1, &digit, 1);
}

void test(int a)
{
    int i = 0;
    while (i < a) {
        write(1, "loop", 4);
        print_digit(i);  // Imprime el i real (0-7)
        write(1, ",", 1);
        i = i + 1;
    }
    write(1, "\n", 1);
}

int main(void)
{
    test(8);
    return 0;
}
