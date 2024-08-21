#pragma once

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t v) {
    __asm__ volatile("outb %b0, %w1" :: "a"(v), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile("inb %w1, %b0" : "=a"(v) : "Nd"(port) : "memory");
    return v;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

static inline uint64_t getCR3(void) {
    uint64_t cr3 = 0;
    __asm__ volatile("mov %%cr3, %%rax \n\t mov %%rax, %0" : "=m"(cr3) :: "%rax");
    return cr3;
}

static inline uint64_t getCR4(void) {
    uint64_t cr4 = 0;
    __asm__ volatile("mov %%cr4, %%rax \n\t mov %%rax, %0" : "=m"(cr4) :: "%rax");
    return cr4;
}
