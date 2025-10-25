void write(int fd, long int s, int n);

void test(int a)
{
    int i = 0;
    while (i < a) {
        write(1, "loop", 4);
        i = i + 1;
    }
    write(1, "\n", 1);
}

int main(void)
{
    test(8);
    return 0;
}
