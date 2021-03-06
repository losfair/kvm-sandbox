#include <unistd.h>

int strlen(const char *s) {
    int len = 0;
    while(*(s++)) len++;
    return len;
}

void __attribute__((section(".loader_start"))) _loader_start() {
    const char *hello_str = "Hello KVM Sandbox\n";
    write(0, hello_str, strlen(hello_str));
    _exit(0);
    while(1) {}
}
