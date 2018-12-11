void __attribute__((section(".loader_start"))) _loader_start() {
    asm volatile(
        "hlt" : : : "memory"
    );
    while(1) {}
}
