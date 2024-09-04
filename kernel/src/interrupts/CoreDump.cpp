#include <cstddef>
#include <cstdint>

#include <interrupts/CoreDump.hpp>
#include <mm/VirtualMemoryLayout.hpp>
#include <screen/Log.hpp>

namespace {
	static inline constexpr uint64_t MAIN_DUMP_USED 		= 0xF1827;
	static inline constexpr uint64_t SECONDARY_DUMP_USED 	= 0x26304;

	static inline constexpr uint64_t PRECISION_4			= 1;
	static inline constexpr uint64_t PRECISION_16			= 4;
	static inline constexpr uint64_t PRECISION_32			= 8;
	static inline constexpr uint64_t DISPLAY_PRECISION		= 16;
	static inline constexpr uint64_t DISPLAY_RADIX			= 16;

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

	extern "C" uint64_t request_dump_type(void);
}

void Panic::dumpCore(uint64_t errv) {
	char buffer[17] = { 0 };

	Log::puts("\n\r------ CORE DUMP ------\n\r");

	CoreDump* dump = nullptr;

	uint64_t dumpType = request_dump_type();
	if (dumpType == SECONDARY_DUMP_USED) {
		dump = reinterpret_cast<CoreDump*>(VirtualMemoryLayout::SECONDARY_CORE_DUMP);	
	}
	else {
		dump = reinterpret_cast<CoreDump*>(VirtualMemoryLayout::MAIN_CORE_DUMP);
	}	

	#define DUMP_REG(REG) do{Log::puts(" "#REG"=0x");_utoax(dump->REG,buffer);Log::puts(buffer);}while(0)
	#define DUMP_REG4(REGA, REGB, REGC, REGD) do{Log::puts("\n\r");DUMP_REG(REGA);DUMP_REG(REGB);DUMP_REG(REGC);DUMP_REG(REGD);}while(0)
	#define DUMP_SS(REG) do{Log::puts("\n\r "#REG" =");_utoax(dump->REG,buffer);Log::puts(buffer);}while(0)

	DUMP_REG4(RAX, RBX, RCX, RDX);
	DUMP_REG4(RSI, RDI, RBP, RSP);

	Log::puts("\n\r R8 =0x");
	_utoax(dump->R8, buffer);
	Log::puts(buffer);
	Log::puts(" R9 =0x");
	_utoax(dump->R9, buffer);
	Log::puts(buffer);
	DUMP_REG(R10);
	DUMP_REG(R11);
	DUMP_REG4(R12, R13, R14, R15);
	Log::puts("\n\r");
	DUMP_REG(RIP);
	DUMP_REG(RFL);

	precision = PRECISION_4;
	Log::puts(" CPL=");
	_utoax(dump->CS & 0x3, buffer);
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
	_utoax(dump->LDTSS, buffer);
	Log::puts(buffer);
	Log::puts("\n\r TR =");
	_utoax(dump->TRSS, buffer);
	Log::puts(buffer);

	precision = DISPLAY_PRECISION;
	Log::puts("\n\r GDT=---- 0x");
	_utoax(dump->GDTBASE, buffer);
	Log::puts(buffer);
	Log::puts(" 0x");
	precision = PRECISION_32;
	_utoax(dump->GDTLIMIT, buffer);
	Log::puts(buffer);

	precision = DISPLAY_PRECISION;
	Log::puts("\n\r IDT=---- 0x");
	_utoax(dump->IDTBASE, buffer);
	Log::puts(buffer);
	Log::puts(" 0x");
	precision = PRECISION_32;
	_utoax(dump->IDTLIMIT, buffer);
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