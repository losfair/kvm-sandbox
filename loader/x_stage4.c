#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>

int state = 42;
char tls[1000];

int main();

asm(
    "do_syscall:\n"
    "int $0x80\n"
    "ret"
);

void do_syscall();

int add_one() {
    return state++;
}

void __attribute__((section(".loader_start"))) _start() {
    int i;
    unsigned int *gs_data = (unsigned int *) 0x80000000;

    for(i = 0; i < 16384; i++) {
        gs_data[i] = 0x7f7f7f7f;
    }

    gs_data[4] = (unsigned int) do_syscall;

    main();

    while(1) {}
    //sprintf(s, "Hello world %d", 42);
    //open("/dev/null", O_RDWR);
}

int main() {
    time(0);
    write(1, "Hello world!\n", 12);
    return 0;
}