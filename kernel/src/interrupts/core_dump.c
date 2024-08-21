#include <stddef.h>
#include <stdint.h>

#include <screen/tty.h>

#define PRECISION4          1
#define PRECISION16         4
#define PRECISION32         8
#define DISPLAY_PRECISION   16
#define DISPLAY_RADIX       16

static unsigned int precision = DISPLAY_PRECISION;

static inline void _utoax(uint64_t x, char buffer[17]) {
    char tmp[16];
    char *tp = tmp;

    uint64_t i;
    uint64_t v = x;

    while (v || tp == tmp) {
        i = v % DISPLAY_RADIX;
        v /= DISPLAY_RADIX;

        if (i < 10) {
            *tp++ = i + u'0';
        } else {
            *tp++ = i + u'a' - 10;
        }
    }

    uint16_t len = tp - tmp;
    while (len < precision) {
        *tp++ = '0';
        ++len;
    }

    while (tp > tmp) {
        *buffer++ = *--tp;
    }
    *buffer++ = '\0';
}

uint8_t core_dump_registers[0x180];

void dump_core(uint64_t errv) {
    tty_puts("\n\r------ CORE DUMP ------\n\r");

    char buffer[17] = { 0 };
    
    const uint64_t RAX = *(uint64_t*)core_dump_registers;
    const uint64_t RBX = *((uint64_t*)core_dump_registers + 1);
    const uint64_t RCX = *((uint64_t*)core_dump_registers + 2);
    const uint64_t RDX = *((uint64_t*)core_dump_registers + 3);
    const uint64_t RSI = *((uint64_t*)core_dump_registers + 4);
    const uint64_t RDI = *((uint64_t*)core_dump_registers + 5);
    const uint64_t RBP = *((uint64_t*)core_dump_registers + 6);
    const uint64_t RSP = *((uint64_t*)core_dump_registers + 7);
    const uint64_t R8  = *((uint64_t*)core_dump_registers + 8);
    const uint64_t R9  = *((uint64_t*)core_dump_registers + 9);
    const uint64_t R10 = *((uint64_t*)core_dump_registers + 10);
    const uint64_t R11 = *((uint64_t*)core_dump_registers + 11);
    const uint64_t R12 = *((uint64_t*)core_dump_registers + 12);
    const uint64_t R13 = *((uint64_t*)core_dump_registers + 13);
    const uint64_t R14 = *((uint64_t*)core_dump_registers + 14);
    const uint64_t R15 = *((uint64_t*)core_dump_registers + 15);
    const uint64_t RIP = *((uint64_t*)core_dump_registers + 16);
    const uint64_t RFL = *((uint64_t*)core_dump_registers + 17);

    const uint16_t ES = *(uint16_t*)((uint64_t*)core_dump_registers + 18);
    const uint16_t CS = *((uint16_t*)((uint64_t*)core_dump_registers + 18) + 1);
    const uint16_t SS = *((uint16_t*)((uint64_t*)core_dump_registers + 18) + 2);
    const uint16_t DS = *((uint16_t*)((uint64_t*)core_dump_registers + 18) + 3);
    const uint16_t FS = *((uint16_t*)((uint64_t*)core_dump_registers + 18) + 4);
    const uint16_t GS = *((uint16_t*)((uint64_t*)core_dump_registers + 18) + 5);

    const uint16_t LDTSS = *(uint16_t*)((uint8_t*)core_dump_registers + 0x0A0);
    const uint16_t TRSS  = *(uint16_t*)((uint8_t*)core_dump_registers + 0x0A2);

    const uint16_t gdtlimit = *(uint16_t*)((uint8_t*)core_dump_registers + 0x0B0);
    const uint64_t gdtbase  = *(uint64_t*)((uint8_t*)core_dump_registers + 0x0B2);
    const uint16_t idtlimit = *(uint16_t*)((uint8_t*)core_dump_registers + 0x0C0);
    const uint64_t idtbase  = *(uint64_t*)((uint8_t*)core_dump_registers + 0x0C2);

    const uint64_t CR0 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x0D0);
    const uint64_t CR2 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x0D8);
    const uint64_t CR3 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x0E0);
    const uint64_t CR4 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x0E8);
    const uint64_t CR8 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x0F0);

    const uint64_t EFER = *(uint64_t*)((uint8_t*)core_dump_registers + 0x0F8);

    const uint64_t DR0 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x100);
    const uint64_t DR1 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x108);
    const uint64_t DR2 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x110);
    const uint64_t DR3 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x118);
    const uint64_t DR6 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x120);
    const uint64_t DR7 = *(uint64_t*)((uint8_t*)core_dump_registers + 0x128);

    #define DUMP_REG(REG) do { tty_puts(" "#REG"=0x");_utoax(REG, buffer);tty_puts(buffer);} while(0)
    #define DUMP_REG4(REG1, REG2, REG3, REG4) do { tty_puts("\n\r"); DUMP_REG(REG1); DUMP_REG(REG2); DUMP_REG(REG3); DUMP_REG(REG4); } while (0)
    #define DUMP_SS(REG) do { tty_puts("\n\r "#REG" =");_utoax(REG, buffer);tty_puts(buffer); } while (0)

    DUMP_REG4(RAX, RBX, RCX, RDX);
    DUMP_REG4(RSI, RDI, RBP, RSP);

    tty_puts("\n\r R8 =0x");
    _utoax(R8, buffer);
    tty_puts(buffer);
    tty_puts(" R9 =0x");
    _utoax(R9, buffer);
    tty_puts(buffer);
    DUMP_REG(R10);
    DUMP_REG(R11);
    DUMP_REG4(R12, R13, R14, R15);
    tty_puts("\n\r");
    DUMP_REG(RIP);
    DUMP_REG(RFL);

    precision = PRECISION4;
    tty_puts(" CPL=");
    _utoax(CS & 0x3, buffer);
    tty_puts(buffer);

    precision = DISPLAY_PRECISION;
    tty_puts(" E=");
    _utoax(errv, buffer);
    tty_puts(buffer);

    precision = PRECISION16;
    DUMP_SS(ES);
    DUMP_SS(CS);
    DUMP_SS(SS);
    DUMP_SS(DS);
    DUMP_SS(FS);
    DUMP_SS(GS);

    tty_puts("\n\r LDT=");
    _utoax(LDTSS, buffer);
    tty_puts(buffer);
    tty_puts("\n\r TR =");
    _utoax(TRSS, buffer);
    tty_puts(buffer);

    precision = DISPLAY_PRECISION;
    tty_puts("\n\r GDT=---- 0x");
    _utoax(gdtbase, buffer);
    tty_puts(buffer);
    tty_puts(" 0x");
    precision = PRECISION32;
    _utoax(gdtlimit, buffer);
    tty_puts(buffer);

    precision = DISPLAY_PRECISION;
    tty_puts("\n\r IDT=---- 0x");
    _utoax(idtbase, buffer);
    tty_puts(buffer);
    tty_puts(" 0x");
    precision = PRECISION32;
    _utoax(idtlimit, buffer);
    tty_puts(buffer);

    precision = DISPLAY_PRECISION;
    DUMP_REG4(CR0, CR2, CR3, CR4);
    tty_puts("\n\r");
    DUMP_REG(CR8);
    DUMP_REG(EFER);

    DUMP_REG4(DR0, DR1, DR2, DR3);
    tty_puts("\n\r");
    DUMP_REG(DR6);
    DUMP_REG(DR7);
    tty_puts("\n\r");
}
