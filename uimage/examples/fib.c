#include <stdio.h>

int fib(int n) {
    if(n == 1 || n == 2) return 1;
    else return fib(n - 1) + fib(n - 2);
}

int main() {
    int x = 42;
    printf("fib(%d) = %d\n", x, fib(x));
    return 0;
}
