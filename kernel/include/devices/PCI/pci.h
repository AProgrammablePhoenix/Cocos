#pragma once

#include <stdint.h>

#pragma pack(push)
#pragma pack(1)
typedef struct {
    uint16_t    VendorID;
    uint16_t    DeviceID;
    uint16_t    Command;
    uint16_t    Status;
    uint8_t     RevisionID;
    uint8_t     ProgrammingInterface;
    uint8_t     SubclassCode;
    uint8_t     BaseClassCode;
    uint8_t     CacheLineSize;
    uint8_t     LatencyTimer;
    uint8_t     HeaderType;
    uint8_t     BIST;
    union {
        uint8_t Raw[8];
        struct {
            union {
                uint32_t BaseAddressRegisters[6];
                uint64_t XBaseAddressRegisters[3];
            };
            uint32_t CardbusCISPointer;
            uint32_t SubsystemVendorID;
            uint32_t SubsystemID;
            uint32_t ExpansionROMBaseAddress;
        } Type0;
        struct {
            union {
                struct {
                    uint32_t    BAR0;
                    uint32_t    BAR1;
                };
                uint64_t XBAR0;
            };
            uint8_t     PrimaryBusNumber;
            uint8_t     SecondaryBusNumber;
            uint8_t     SubordinateBusNumber;
            uint8_t     SecondaryLatencyTimer;
            uint8_t     IOBase;
            uint8_t     IOLimit;
            uint16_t    SecondaryStatus;
            uint16_t    MemoryBase;
            uint16_t    MemoryLimit;
            uint16_t    PrefetchableMemoryBase;
            uint16_t    PrefetchableMemoryLimit;
            uint32_t    PrefetchableBaseUpper32;
            uint32_t    PrefetchableLimitUpper32;
            uint16_t    IOBaseUpper16;
            uint16_t    IOLimitUpper16;
        } Type1;
    } TypeSpecificData1;
    uint8_t CapabilitiesPointer;
    union {
        uint8_t Raw[7];
        struct {
            uint8_t     Reserved[3];
            uint32_t    ExpansionROMBaseAddress;
        } Type1;
    } TypeSpecificData2;
    uint8_t InterruptLine;
    uint8_t InterruptPin;
    union {
        struct {
            uint8_t Min_Gnt;
            uint8_t Max_Lat;
        } Type0;
        struct {
            uint16_t BridgeControl;
        } Type1;
    } TypeSpecificData3;
} PCI_CS;
#pragma pack(pop)

