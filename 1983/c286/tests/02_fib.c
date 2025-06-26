int main()
{
        register a, b;
        a = 1;
        b = 0;
        while (b < 1024) {
                register tmp = a;
                printf("%d\n", b);
                a = a + b;
                b = tmp;
        }
        return(0);
}
