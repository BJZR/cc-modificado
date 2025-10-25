void write(int fd, long int s, int n);

void test(int a)
{
    int i = 0;
    while (i < a) {  // Loop mientras i < a (8 veces)
        write(1, "i=", 2);
        write(1, "0", 1);  // Hardcode "0" por simplicidad, len=1
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
