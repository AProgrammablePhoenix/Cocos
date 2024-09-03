#pragma once

#include <stdint.h>

#pragma pack(push)
#pragma pack(1)
struct ACPI_GAS {
    enum : uint8_t {
        SystemMemory            = 0x00,
        SystemIO                = 0x01,
        PCIConfiguration        = 0x02,
        EmbeddedController      = 0x03,
        SMBus                   = 0x04,
        SystemCMOS              = 0x05,
        PciBarTaget             = 0x06,
        IPMI                    = 0x07,
        GeneralPurposeIO        = 0x08,
        GenericSerialBus        = 0x09,
        PCC                     = 0x0A,
        FunctionalFixedHardware = 0x7F
    } AddressSpaceID;
    uint8_t RegisterBitWidth;
    uint8_t RegisterBitOffset;
    enum : uint8_t {
        Undefined   = 0,
        ByteAccess  = 1,
        WordAccess  = 2,
        DwordAccess = 3,
        QwordAccess = 4
    } AccessSize;
    uint64_t Address;
};

#define LEGACY_RSDP_SIZE    20

struct ACPI_RSDP {
    uint8_t     Signature[8];
    uint8_t     Checksum;
    uint8_t     OEMID[6];
    uint8_t     Revision;
    uint32_t    RsdtAddress;
    uint32_t    Length;
    uint64_t    XsdtAddress;
    uint8_t     ExtendedChecksum;
    uint8_t     Reserved[3];
};

struct ACPI_SDTH {
    uint8_t     Signature[4];
    uint32_t    Length;
    uint8_t     Revision;
    uint8_t     Checksum;
    uint8_t     OEMID[6];
    uint8_t     OEMTableID[8];
    uint32_t    OEMRevision;
    uint8_t     CreatorID[4];
    uint32_t    CreatorRevision;
};

struct ACPI_RSDT {
    ACPI_SDTH   Header;
    uint32_t    Entry; // first entry
};

struct ACPI_XSDT {
    ACPI_SDTH   Header;
    uint64_t    Entry;
};

// FADT Flags masks
#define WBINVD                                  0x00000001
#define WBINVD_FLUSH                            0x00000002
#define PROC_C1                                 0x00000004
#define P_LVL2_UP                               0x00000008
#define PWR_BUTTON                              0x00000010
#define SLP_BUTTON                              0x00000020
#define FIX_RTC                                 0x00000040
#define RTC_S4                                  0x00000080
#define TMR_VAL_EXT                             0x00000100
#define DCK_CAP                                 0x00000200
#define RESET_REG_SUP                           0x00000400
#define SEALED_CASE                             0x00000800
#define HEADLESS                                0x00001000
#define CPU_SW_SLP                              0x00002000
#define PCI_EXP_WAK                             0x00004000
#define USE_PLATFORM_CLOCK                      0x00008000
#define S4_RTC_STS_VALID                        0x00010000
#define REMOTE_POWER_ON_CAPABLE                 0x00020000
#define FORCE_APIC_CLUSTER_MODEL                0x00040000
#define FORCE_APIC_PHYSICAL_DESTINATION_MODE    0x00080000
#define HW_REDUCED_ACPI                         0x00100000
#define LOW_POWER_S0_IDLE_CAPABLE               0x00200000

// FADT IAPC_BOOT_ARCH masks
#define LEGACY_DEVICES          0x0001
#define _8042                   0x0002
#define VGA_NOT_PRESENT         0x0004
#define MSI_NOT_SUPPORTED       0x0008
#define PCIE_ASPM_CONTROLS      0x0010
#define CMOS_RTC_NOT_PRESENT    0x0020

// FADT ARM_BOOT_ARCH masks
#define PSCI_COMPLIANT          0x0001
#define PSCI_USE_HVC            0x0002

struct ACPI_FADT {
    ACPI_SDTH   Header;
    uint32_t    FIRMWARE_CTRL;
    uint32_t    DSDT;
    uint8_t     Reserved_1;
    enum : uint8_t {
        Unspecified         = 0,
        Desktop             = 1,
        Mobile              = 2,
        Workstation         = 3,
        EnterpriseServer    = 4,
        SOHOServer          = 4,
        AppliancePC         = 5,
        PerformanceServer   = 6,
        Tablet              = 7
    } Preferred_PM_Profile;
    uint16_t    SCI_INT;
    uint32_t    SMI_CMD;
    uint8_t     ACPI_ENABLE;
    uint8_t     ACPI_DISABLE;
    uint8_t     S4BIOS_REQ;
    uint8_t     PSTATE_CNT;
    uint32_t    PM1a_EVT_BLK;
    uint32_t    PM1b_EVT_BLK;
    uint32_t    PM1a_CNT_BLK;
    uint32_t    PM1b_CNT_BLK;
    uint32_t    PM2_CNT_BLK;
    uint32_t    PM_TMR_BLK;
    uint32_t    GPE0_BLK;
    uint32_t    GPE1_BLK;
    uint8_t     PM1_EVT_LEN;
    uint8_t     PM1_CNT_LEN;
    uint8_t     PM2_CNT_LEN;
    uint8_t     PM_TMR_LEN;
    uint8_t     GPE0_BLK_LEN;
    uint8_t     GPE1_BLK_LEN;
    uint8_t     GPE1_BASE;
    uint8_t     CST_CNT;
    uint16_t    P_LVL2_LAT;
    uint16_t    P_LVL3_LAT;
    uint16_t    FLUSH_SIZE;
    uint16_t    FLUSH_STRIDE;
    uint8_t     DUTY_OFFSET;
    uint8_t     DUTY_WIDTH;
    uint8_t     DAY_ALRM;
    uint8_t     MON_ALRM;
    uint8_t     CENTURY;
    uint16_t    IAPC_BOOT_ARCH;
    uint8_t     Reserved_2;
    uint32_t    Flags;
    ACPI_GAS    RESET_REG;
    uint8_t     RESET_VALUE;
    uint16_t    ARM_BOOT_ARCH;
    uint8_t     FADTMinorVersion;
    uint64_t    X_FIRMWARE_CTRL;
    uint64_t    X_DSDT;
    ACPI_GAS    X_PM1a_EVT_BLK;
    ACPI_GAS    X_PM1b_EVT_BLK;
    ACPI_GAS    X_PM1a_CNT_BLK;
    ACPI_GAS    X_PM1b_CNT_BLK;
    ACPI_GAS    X_PM2_CNT_BLK;
    ACPI_GAS    X_PM_TMR_BLK;
    ACPI_GAS    X_GPE0_BLK;
    ACPI_GAS    X_GPE1_BLK;
    ACPI_GAS    SLEEP_CONTROL_REG;
    ACPI_GAS    SLEEP_STATUS_REG;
    uint64_t    HypervisorVendorIdentity;
};

struct PCI_CSBA {
    uint64_t    BaseAddress;
    uint16_t    PCISegmentGroupNumber;
    uint8_t     StartBusNumber;
    uint8_t     EndBusNumber;
    uint32_t    Reserved;
};

struct ACPI_MCFG {
    ACPI_SDTH   Header;
    uint64_t    Reserved;
    PCI_CSBA    Entry;      // first entry
};

struct PCI_CS {
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
                } BARS;
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
};
#pragma pack(pop)
