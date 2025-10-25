void write(int fd, long int s, int n);

void test(int a)
{
    int x = a;
    int y = x + 5;  // y=13
    write(1, "y=", 2);

    if (y > 10) {       // Condición: binary OP_GREATER
        write(1, "big\n", 4);  // Solo si true
    }
}

int main(void)
{
    test(8);
    return 0;
}
