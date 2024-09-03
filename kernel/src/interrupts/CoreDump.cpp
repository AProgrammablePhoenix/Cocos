#include <cstddef>
#include <cstdint>

#include <interrupts/CoreDump.hpp>
#include <screen/Log.hpp>

namespace {
	static inline constexpr uint64_t PRECISION_4		= 1;
	static inline constexpr uint64_t PRECISION_16		= 4;
	static inline constexpr uint64_t PRECISION_32		= 8;
	static inline constexpr uint64_t DISPLAY_PRECISION	= 16;
	static inline constexpr uint64_t DISPLAY_RADIX		= 16;

	static unsigned int precision = DISPLAY_PRECISION;

	static inline void _utoax(uint64_t x, char buffer[17]) {
		char tmp[16] = { 0 };
		char* tp = tmp;

		uint64_t i;
		uint64_t v = x;

		while (v || tp == tmp) {
			i = v % DISPLAY_RADIX;
			v /= DISPLAY_RADIX;

			if (i < 10) {
				*tp++ = i + '0';
			}
			else {
				*tp++ = i + 'a' - 10;
			}
		}

		uint64_t len = tp - tmp;
		while (len < precision) {
			*tp++ = '0';
			++len;
		}
		while (tp > tmp) {
			*buffer++ = *--tp;
		}
		*buffer++ = '\0';
	}
}

extern "C" {
	uint8_t core_dump_registers[0x180];
}

void Panic::dumpCore(uint64_t errv) {
	char buffer[17] = { 0 };

	Log::puts("\n\r------ CORE DUMP ------\n\r");

	static uint64_t* const basePtr		= reinterpret_cast<uint64_t*>(core_dump_registers);
	static uint16_t* const segmentsPtr	= reinterpret_cast<uint16_t*>(basePtr + 18);
	static uint64_t* const extendedPtr	= reinterpret_cast<uint64_t*>(core_dump_registers + 0xD0);

	const uint64_t RAX = *basePtr;
	const uint64_t RBX = *(basePtr + 1);
	const uint64_t RCX = *(basePtr + 2);
	const uint64_t RDX = *(basePtr + 3);
	const uint64_t RSI = *(basePtr + 4);
	const uint64_t RDI = *(basePtr + 5);
	const uint64_t RBP = *(basePtr + 6);
	const uint64_t RSP = *(basePtr + 7);
	const uint64_t R8  = *(basePtr + 8);
	const uint64_t R9  = *(basePtr + 9);
	const uint64_t R10 = *(basePtr + 10);
	const uint64_t R11 = *(basePtr + 11);
	const uint64_t R12 = *(basePtr + 12);
	const uint64_t R13 = *(basePtr + 13);
	const uint64_t R14 = *(basePtr + 14);
	const uint64_t R15 = *(basePtr + 15);
	const uint64_t RIP = *(basePtr + 16);
	const uint64_t RFL = *(basePtr + 17);

	const uint16_t ES = *segmentsPtr;
	const uint16_t CS = *(segmentsPtr + 1);
	const uint16_t SS = *(segmentsPtr + 2);
	const uint16_t DS = *(segmentsPtr + 3);
	const uint16_t FS = *(segmentsPtr + 4);
	const uint16_t GS = *(segmentsPtr + 5);

	const uint16_t LDTSS	= *reinterpret_cast<uint16_t*>(core_dump_registers + 0xA0);
	const uint16_t TRSS		= *reinterpret_cast<uint16_t*>(core_dump_registers + 0xA2);

	const uint16_t gdtLimit = *reinterpret_cast<uint16_t*>(core_dump_registers + 0xB0);
	const uint64_t gdtBase	= *reinterpret_cast<uint64_t*>(core_dump_registers + 0xB2);
	const uint16_t idtLimit = *reinterpret_cast<uint16_t*>(core_dump_registers + 0xC0);
	const uint64_t idtBase	= *reinterpret_cast<uint64_t*>(core_dump_registers + 0xC2);

	const uint64_t CR0 = *(extendedPtr);
	const uint64_t CR2 = *(extendedPtr + 1);
	const uint64_t CR3 = *(extendedPtr + 2);
	const uint64_t CR4 = *(extendedPtr + 3);
	const uint64_t CR8 = *(extendedPtr + 4);

	const uint64_t EFER = *(extendedPtr + 5);

	const uint64_t DR0 = *(extendedPtr + 6);
	const uint64_t DR1 = *(extendedPtr + 7);
	const uint64_t DR2 = *(extendedPtr + 8);
	const uint64_t DR3 = *(extendedPtr + 9);
	const uint64_t DR6 = *(extendedPtr + 10);
	const uint64_t DR7 = *(extendedPtr + 11);

	#define DUMP_REG(REG) do{Log::puts(" "#REG"=0x");_utoax(REG,buffer);Log::puts(buffer);}while(0)
	#define DUMP_REG4(REGA, REGB, REGC, REGD) do{Log::puts("\n\r");DUMP_REG(REGA);DUMP_REG(REGB);DUMP_REG(REGC);DUMP_REG(REGD);}while(0)
	#define DUMP_SS(REG) do{Log::puts("\n\r "#REG" =");_utoax(REG,buffer);Log::puts(buffer);}while(0)

	DUMP_REG4(RAX, RBX, RCX, RDX);
	DUMP_REG4(RSI, RDI, RBP, RSP);

	Log::puts("\n\r R8 =0x");
	_utoax(R8, buffer);
	Log::puts(buffer);
	Log::puts(" R9 =0x");
	_utoax(R9, buffer);
	Log::puts(buffer);
	DUMP_REG(R10);
	DUMP_REG(R11);
	DUMP_REG4(R12, R13, R14, R15);
	Log::puts("\n\r");
	DUMP_REG(RIP);
	DUMP_REG(RFL);

	precision = PRECISION_4;
	Log::puts(" CPL=");
	_utoax(CS & 0x3, buffer);
	Log::puts(buffer);

	precision = DISPLAY_PRECISION;
	Log::puts(" E=");
	_utoax(errv, buffer);
	Log::puts(buffer);

	precision = PRECISION_16;
	DUMP_SS(ES);
	DUMP_SS(CS);
	DUMP_SS(SS);
	DUMP_SS(DS);
	DUMP_SS(FS);
	DUMP_SS(GS);
	
	Log::puts("\n\r LDT=");
	_utoax(LDTSS, buffer);
	Log::puts(buffer);
	Log::puts("\n\r TR =");
	_utoax(TRSS, buffer);
	Log::puts(buffer);

	precision = DISPLAY_PRECISION;
	Log::puts("\n\r GDT=---- 0x");
	_utoax(gdtBase, buffer);
	Log::puts(buffer);
	Log::puts(" 0x");
	precision = PRECISION_32;
	_utoax(gdtLimit, buffer);
	Log::puts(buffer);

	precision = DISPLAY_PRECISION;
	Log::puts("\n\r IDT=---- 0x");
	_utoax(idtBase, buffer);
	Log::puts(buffer);
	Log::puts(" 0x");
	precision = PRECISION_32;
	_utoax(idtLimit, buffer);
	Log::puts(buffer);

	precision = DISPLAY_PRECISION;
	DUMP_REG4(CR0, CR2, CR3, CR4);
	Log::puts("\n\r");
	DUMP_REG(CR8);
	DUMP_REG(EFER);

	DUMP_REG4(DR0, DR1, DR2, DR3);
	Log::puts("\n\r");
	DUMP_REG(DR6);
	DUMP_REG(DR7);
	Log::puts("\n\r");
}