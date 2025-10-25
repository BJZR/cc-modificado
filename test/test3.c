void write(int fd, long int s, int n);

void test(int a)
{
    int x = a;  // Local: prueba store/load en offset negativo
    write(1, "8", x);  // Usa x (debería imprimir "8" si x=8)
    write(1, "\n", 1);
}
/*
 * loco
 */
int main(void)
{
    test(3 + 5);  // Binary op: 8 en %rdi
    return 0;
}
