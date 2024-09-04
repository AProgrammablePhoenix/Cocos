#pragma once

#include <cstdint>

namespace Panic {
#pragma pack(push)
#pragma pack(1)

	struct CoreDump {
		uint64_t RAX;		// 0x00
		uint64_t RBX;		// 0x08
		uint64_t RCX;		// 0x10
		uint64_t RDX;		// 0x18
		uint64_t RSI;		// 0x20
		uint64_t RDI;		// 0x28
		uint64_t RBP;		// 0x30
		uint64_t RSP;		// 0x38
		uint64_t R8;		// 0x40
		uint64_t R9;		// 0x48
		uint64_t R10;		// 0x50
		uint64_t R11;		// 0x58
		uint64_t R12;		// 0x60
		uint64_t R13;		// 0x68
		uint64_t R14;		// 0x70
		uint64_t R15;		// 0x78
		uint64_t RIP;		// 0x80
		uint64_t RFL;		// 0x88
		uint16_t ES;		// 0x90
		uint16_t CS;		// 0x92
		uint16_t SS;		// 0x94
		uint16_t DS;		// 0x96
		uint16_t FS;		// 0x98
		uint16_t GS;		// 0x9A
		uint16_t LDTSS;		// 0x9C
		uint16_t TRSS;		// 0x9E
		uint16_t GDTLIMIT;  // 0xA0
		uint16_t IDTLIMIT;	// 0xA2
		uint32_t _padding;	// 0xA4
		uint64_t GDTBASE;	// 0xA8
		uint64_t IDTBASE;	// 0xB0
		uint64_t CR0;		// 0xB8
		uint64_t CR2;		// 0xC0
		uint64_t CR3;		// 0xC8
		uint64_t CR4; 		// 0xD0
		uint64_t CR8;		// 0xD8
		uint64_t EFER;		// 0xE0
		uint64_t DR0;		// 0xE8
		uint64_t DR1;		// 0xF0
		uint64_t DR2;		// 0xF8
		uint64_t DR3;		// 0x100
		uint64_t DR6;		// 0x108
		uint64_t DR7;		// 0x110
	};

#pragma pack(pop)

	void dumpCore(uint64_t errv);
}
