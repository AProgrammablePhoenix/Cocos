#pragma once

#include <stdint.h>

#include <efi/efi_datatypes.h>

#define SIGNATURE_OFFSET 0x3C

static inline int check_signature(uint8_t* fc) {
    return (*(fc + SIGNATURE_OFFSET) == (uint8_t)'P')
            && (*(fc + SIGNATURE_OFFSET + 1) == (uint8_t)'E')
            && (*(fc + SIGNATURE_OFFSET + 2) == 0)
            && (*(fc + SIGNATURE_OFFSET + 3) == 0);
}

typedef struct {
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} COFF_FH;

typedef struct {
    struct {
        uint16_t Magic;
        uint8_t MajorLinkerVersion;
        uint8_t MinorLinkerVersion;
        uint32_t SizeOfCode;
        uint32_t SizeOfInitializedData;
        uint32_t SizeOfUninitializedData;
        uint32_t AddressOfEntryPoint;
        uint32_t BaseOfCode;
    } StandardFields;

    struct {
        uint64_t ImageBase;
        uint32_t SectionAlignment;
        uint32_t FileAlignment;
        uint16_t MajorOperatingSystemVersion;
        uint16_t MinorOperatingSystemVersion;
        uint16_t MajorImageVersion;
        uint16_t MinorImageVersion;
        uint16_t MajorSubsystemVersion;
        uint16_t MinorSubsystemVersion;
        uint32_t Win32VersionValue;
        uint32_t SizeOfImage;
        uint32_t SizeOfHeaders;
        uint32_t CheckSum;
        uint16_t Subsystem;
        uint16_t DllCharacteristics;
        uint64_t SizeOfStackReserve;
        uint64_t SizeOfStackCommit;
        uint64_t SizeOfHeapReserve;
        uint64_t SizeOfHeapCommit;
        uint32_t LoaderFlags;
        uint32_t NumberOfRvaAndSizes;
    } WindowsSpecificFields;
} OPT_HEADER;

typedef struct {
    uint32_t VirtualAddress;
    uint32_t Size;
} DATA_DIRETORY;

typedef struct {
    uint8_t Name[8];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
} SECTION_HEADER;

typedef struct {
    uint8_t* CurrentAddress;
    uint64_t SizeOfImage;
    uint64_t ImageBase;
    uint64_t RelativeEntryPoint;
} KernelLocInfo;

KernelLocInfo loadKernel(EFI_HANDLE ImageHandle);
