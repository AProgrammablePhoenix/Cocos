#pragma once

#ifndef __EFI_STANDALONE__

#include <efi/efi_datatypes.h>

#else

#include <stdint.h>
#include <uchar.h>

typedef uint8_t                 BOOLEAN;
typedef int64_t                 INTN;
typedef uint64_t                UINTN;
typedef int8_t                  INT8;
typedef uint8_t                 UINT8;
typedef int16_t                 INT16;
typedef uint16_t                UINT16;
typedef int32_t                 INT32;
typedef uint32_t                UINT32;
typedef int64_t                 INT64;
typedef uint64_t                UINT64;
typedef _BitInt(128)            INT128;
typedef unsigned _BitInt(128)   UINT128;
typedef uint8_t                 CHAR8;
typedef char16_t                CHAR16;
typedef void                    VOID;

typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8  Data4[8];
} EFI_GUID;

typedef UINTN EFI_STATUS;
typedef VOID* EFI_HANDLE;
typedef VOID* EFI_EVENT;
typedef UINT64 EFI_LBA;
typedef UINTN EFI_TPL;

typedef struct {
    UINT8 Addr[32];
} EFI_MAC_ADDRESS;

typedef struct {
    UINT8 Addr[4];
} EFI_IPv4_ADDRESS;

typedef struct {
    UINT8 Addr[16];
} EFI_IPv6_ADDRESS;

typedef union {
    UINT32 Addr[4];
    EFI_IPv4_ADDRESS v4;
    EFI_IPv6_ADDRESS v6;
} EFI_IP_ADDRESS;

typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef UINT64 EFI_VIRTUAL_ADDRESS;

typedef UINTN EFI_TPL;

#endif

#define IN
#define OUT
#define OPTIONAL
#define CONST const

#define EFIAPI __attribute__((ms_abi))

#define EFI_WARN_MASK               0x0000000000000000ULL
#define EFI_ERROR_MASK              0x8000000000000000ULL

#define EFI_SUCCESS                 0
#define EFI_LOAD_ERROR              (EFI_ERROR_MASK | 1)
#define EFI_INVALID_PARAMETER       (EFI_ERROR_MASK | 2)
#define EFI_UNSUPPORTED             (EFI_ERROR_MASK | 3)
#define EFI_BAD_BUFFER_SIZE         (EFI_ERROR_MASK | 4)
#define EFI_BUFFER_TOO_SMALL        (EFI_ERROR_MASK | 5)
#define EFI_NOT_READY               (EFI_ERROR_MASK | 6)
#define EFI_DEVICE_ERROR            (EFI_ERROR_MASK | 7)
#define EFI_WRITE_PROTECTED         (EFI_ERROR_MASK | 8)
#define EFI_OUT_OF_RESOURCES        (EFI_ERROR_MASK | 9)
#define EFI_VOLUME_CORRUPTED        (EFI_ERROR_MASK | 10)
#define EFI_VOLUME_FULL             (EFI_ERROR_MASK | 11)
#define EFI_NO_MEDIA                (EFI_ERROR_MASK | 12)
#define EFI_MEDIA_CHANGED           (EFI_ERROR_MASK | 13)
#define EFI_NOT_FOUND               (EFI_ERROR_MASK | 14)
#define EFI_ACCESS_DENIED           (EFI_ERROR_MASK | 15)
#define EFI_NO_RESPONSE             (EFI_ERROR_MASK | 16)
#define EFI_NO_MAPPING              (EFI_ERROR_MASK | 17)
#define EFI_TIMEOUT                 (EFI_ERROR_MASK | 18)
#define EFI_NOT_STARTED             (EFI_ERROR_MASK | 19)
#define EFI_ALREADY_STARTED         (EFI_ERROR_MASK | 20)
#define EFI_ABORTED                 (EFI_ERROR_MASK | 21)
#define EFI_ICMP_ERROR              (EFI_ERROR_MASK | 22)
#define EFI_TFTP_ERROR              (EFI_ERROR_MASK | 23)
#define EFI_PROTOCOL_ERROR          (EFI_ERROR_MASK | 24)
#define EFI_INCOMPATIBLE_VERSION    (EFI_ERROR_MASK | 25)
#define EFI_SEURITY_VIOLATION       (EFI_ERROR_MASK | 26)
#define EFI_CRC_ERROR               (EFI_ERROR_MASK | 27)
#define EFI_END_OF_MEDIA            (EFI_ERROR_MASK | 28)
#define EFI_OF_FILE                 (EFI_ERROR_MASK | 31)
#define EFI_INVALID_LANGUAGE        (EFI_ERROR_MASK | 32)
#define EFI_COMPROMISED_DATA        (EFI_ERROR_MASK | 33)
#define EFI_IP_ADDRESS_CONFLICT     (EFI_ERROR_MASK | 34)
#define EFI_HTTP_ERROR              (EFI_ERROR_MASK | 35)
#define EFI_WARN_UNKNOWN_GLYPH      (EFI_WARN_MASK | 1)
#define EFI_WARN_DELETE_FAILURE     (EFI_WARN_MASK | 2)
#define EFI_WARN_WRITE_FAILURE      (EFI_WARN_MASK | 3)
#define EFI_WARN_BUFFER_TOO_SMALL   (EFI_WARN_MASK | 4)
#define EFI_WARN_STALE_DATA         (EFI_WARN_MASK | 5)
#define EFI_WARN_FILE_SYSTEM        (EFI_WARN_MASK | 6)
#define EFI_WARN_RESET_REQUIRED     (EFI_WARN_MASK | 7)

typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

typedef struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef struct {
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET) (IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, IN BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY) (IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, OUT EFI_INPUT_KEY *Key);

struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_INPUT_RESET         Reset;
    EFI_INPUT_READ_KEY      ReadKeyStroke;
    EFI_EVENT               WaitForKey;
};

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET) (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING) (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN CHAR16 *String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_TEST_STRING) (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN CHAR16 *String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_QUERY_MODE) (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN UINTN ModeNumber, OUT UINTN* Columns, OUT UINTN* Rows);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_MODE) (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN UINTN ModeNumber);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE) (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN UINTN Attribute);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN) (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_CURSOR_POSITION) (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN UINTN Column, IN UINTN Row);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_ENABLE_CURSOR) (IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN BOOLEAN Visible);

typedef struct {
    INT32 MaxMode;
    INT32 Mode;
    INT32 Attribute;
    INT32 CursorColumn;
    INT32 CursorRow;
    BOOLEAN CursorVisible;
} SIMPLE_TEXT_OUTPUT_MODE;

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET                  Reset;
    EFI_TEXT_STRING                 OutputString;
    EFI_TEXT_TEST_STRING            TestString;
    EFI_TEXT_QUERY_MODE             QueryMode;
    EFI_TEXT_SET_MODE               SetMode;
    EFI_TEXT_SET_ATTRIBUTE          SetAttribute;
    EFI_TEXT_CLEAR_SCREEN           ClearScreen;
    EFI_TEXT_SET_CURSOR_POSITION    SetCursorPosition;
    EFI_TEXT_ENABLE_CURSOR          EnableCursor;
    SIMPLE_TEXT_OUTPUT_MODE         *Mode;
};

typedef struct {
    UINT16  Year;
    UINT8   Month;
    UINT8   Day;
    UINT8   Hour;
    UINT8   Minute;
    UINT8   Second;
    UINT8   Pad1;
    UINT32  Nanoseconds;
    UINT16  TimeZone;
    UINT8   Daylight;
    UINT8   Pad2;
} EFI_TIME;

typedef struct {
    UINT32  Resolution;
    UINT32  Accuracy;
    BOOLEAN SetsToZero;
} EFI_TIME_CAPABILITIES;

typedef EFI_STATUS (EFIAPI *EFI_GET_TIME) (OUT EFI_TIME *Time, OUT EFI_TIME_CAPABILITIES *Capabilities OPTIONAL);
typedef EFI_STATUS (EFIAPI *EFI_SET_TIME) (IN EFI_TIME *Time);
typedef EFI_STATUS (EFIAPI *EFI_GET_WAKEUP_TIME) (OUT BOOLEAN *Enabled, OUT BOOLEAN *Pending, OUT EFI_TIME *Time);
typedef EFI_STATUS (EFIAPI *EFI_SET_WAKEUP_TIME) (IN BOOLEAN Enable, IN EFI_TIME *Time OPTIONAL);

typedef struct EFI_MEMORY_DESCRIPTOR EFI_MEMORY_DESCRIPTOR;

typedef EFI_STATUS (EFIAPI *EFI_SET_VIRTUAL_ADDRESS_MAP) (IN UINTN MemoryMapSize, IN UINTN DescriptorSize, IN UINT32 DescriptorVersion, IN EFI_MEMORY_DESCRIPTOR *VirtualMap);
typedef EFI_STATUS (EFIAPI *EFI_CONVERT_POINTER) (IN UINTN DebugDisposition, IN VOID **Address);

typedef EFI_STATUS (EFIAPI *EFI_GET_VARIABLE) (
    IN CHAR16       *VariableName,
    IN EFI_GUID     *VendorGuid,
    OUT UINT32      *Attributes OPTIONAL,
    IN OUT UINTN    *DataSize,
    OUT VOID        *Data OPTIONAL
);
typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_VARIABLE_NAME) (IN OUT UINTN *VariableNameSize, IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid);
typedef EFI_STATUS (EFIAPI *EFI_SET_VARIABLE) (
    IN CHAR16       *VariableName,
    IN EFI_GUID     *VendorGuid,
    IN UINT32       Attributes,
    IN UINTN        DataSize,
    IN VOID         *Data
);

typedef enum {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown,
    EfiResetPlatformSpecific
} EFI_RESET_TYPE;

typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_HIGH_MONO_COUNT) (OUT UINT32 *HighCount);
typedef EFI_STATUS (EFIAPI *EFI_RESET_SYSTEM) (IN EFI_RESET_TYPE ResetType, IN EFI_STATUS ResetStatus, IN UINTN DataSize, IN VOID *ResetData OPTIONAL);

typedef struct {
    EFI_GUID CapsuleGuid;
    UINT32 HeaderSize;
    UINT32 Flags;
    UINT32 CapsuleImageSize;
} EFI_CAPSULE_HEADER;

typedef EFI_STATUS (EFIAPI *EFI_UPDATE_CAPSULE) (
    IN EFI_CAPSULE_HEADER   **CapsuleHeaderArray,
    IN UINTN                CapsuleCount,
    IN EFI_PHYSICAL_ADDRESS ScatterGatherList OPTIONAL
);
typedef EFI_STATUS (EFIAPI *EFI_QUERY_CAPSULE_CAPABILITIES) (
    IN EFI_CAPSULE_HEADER   **CapsuleHeaderArray,
    IN UINTN                CapsuleCount,
    OUT UINT64              *MaximumCapsuleSize,
    OUT EFI_RESET_TYPE      *ResetType
);

typedef EFI_STATUS (EFIAPI *EFI_QUERY_VARIABLE_INFO) (
    IN UINT32   Attributes,
    OUT UINT64  *MaximumVariableStorageSize,
    OUT UINT64  *RemainingVariableStorageSize,
    OUT UINT64  *MaximumVariableSize
);

typedef struct {
    EFI_TABLE_HEADER                Hdr;

    EFI_GET_TIME                    GetTime;
    EFI_SET_TIME                    SetTime;
    EFI_GET_WAKEUP_TIME             GetWakeupTime;
    EFI_SET_WAKEUP_TIME             SetWakeupTime;

    EFI_SET_VIRTUAL_ADDRESS_MAP     SetVirtualAddressMap;
    EFI_CONVERT_POINTER             ConvertPointer;

    EFI_GET_VARIABLE                GetVariable;
    EFI_GET_NEXT_VARIABLE_NAME      GetNextVariableName;
    EFI_SET_VARIABLE                SetVariable;

    EFI_GET_NEXT_HIGH_MONO_COUNT    GetNextHighMonotonicCount;
    EFI_RESET_SYSTEM                ResetSystem;

    EFI_UPDATE_CAPSULE              UpdateCapsule;
    EFI_QUERY_CAPSULE_CAPABILITIES  QueryCapsuleCapabilities;

    EFI_QUERY_VARIABLE_INFO         QueryVariableinfo;
} EFI_RUNTIME_SERVICES;

typedef EFI_TPL (EFIAPI *EFI_RAISE_TPL) (IN EFI_TPL NewTpl);
typedef VOID (EFIAPI *EFI_RESTORE_TPL) (IN EFI_TPL OldTpl);

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum : uint32_t {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,

    LoaderPersistentMemory  = 0x80000000,
    LoaderTemporaryMemory   = 0x80000001
} EFI_MEMORY_TYPE;

struct EFI_MEMORY_DESCRIPTOR {
    UINT32 Type;
    UINT32 _padding;
    EFI_PHYSICAL_ADDRESS PhysicalStart;
    EFI_VIRTUAL_ADDRESS VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
};

typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_PAGES) (
    IN EFI_ALLOCATE_TYPE        Type,
    IN EFI_MEMORY_TYPE          MemoryType,
    IN UINTN                    Pages,
    IN OUT EFI_PHYSICAL_ADDRESS *Memory
);
typedef EFI_STATUS (EFIAPI *EFI_FREE_PAGES) (IN EFI_PHYSICAL_ADDRESS Memory, IN UINTN Pages);
typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP) (
    IN OUT UINTN                *MemoryMapSize,
    OUT EFI_MEMORY_DESCRIPTOR   *MemoryMap,
    OUT UINTN                   *MapKey,
    OUT UINTN                   *DescriptorSize,
    OUT UINT32                  *DescriptorVersion
);
typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL) (IN EFI_MEMORY_TYPE PoolType, IN UINTN Size, OUT VOID **Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FREE_POOL) (IN VOID* Buffer);

typedef VOID (EFIAPI *EFI_EVENT_NOTIFY) (IN EFI_EVENT Event, IN VOID *Context);
typedef enum {
    TimerCancel,
    TimerPeriodic,
    TimerRelative
} EFI_TIMER_DELAY;

typedef EFI_STATUS (EFIAPI *EFI_CREATE_EVENT) (
    IN UINT32               Type,
    IN EFI_TPL              NotifyTpl,
    IN EFI_EVENT_NOTIFY     NotifyFunction,
    IN VOID                 *NotifyContext OPTIONAL,
    OUT EFI_EVENT           *Event OPTIONAL
);
typedef EFI_STATUS (EFIAPI *EFI_SET_TIMER) (IN EFI_EVENT Event, IN EFI_TIMER_DELAY Type, IN UINT64 TriggerTime);
typedef EFI_STATUS (EFIAPI *EFI_WAIT_FOR_EVENT) (IN UINTN NumberOfEvents, IN EFI_EVENT *Event, OUT UINTN *Index);
typedef EFI_STATUS (EFIAPI *EFI_SIGNAL_EVENT) (IN EFI_EVENT Event);
typedef EFI_STATUS (EFIAPI *EFI_CLOSE_EVENT) (IN EFI_EVENT Event);
typedef EFI_STATUS (EFIAPI *EFI_CHECK_EVENT) (IN EFI_EVENT Event);

typedef enum {
    EFI_NATIVE_INTERFACE
} EFI_INTERFACE_TYPE;
typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;
typedef struct {
    UINT8 Type;
    UINT8 SubType;
    UINT8 Length[2];
} EFI_DEVICE_PATH_PROTOCOL;


typedef EFI_STATUS (EFIAPI *EFI_INSTALL_PROTOCOL_INTERFACE) (
    IN OUT EFI_HANDLE       *Handle,
    IN EFI_GUID             *Protocol,
    IN EFI_INTERFACE_TYPE   InterfaceType,
    IN VOID                 *Interface
);
typedef EFI_STATUS (EFIAPI *EFI_REINSTALL_PROTOCOL_INTERFACE) (
    IN EFI_HANDLE           Handle,
    IN EFI_GUID             *Protocol,
    IN VOID                 *OldInterface,
    IN VOID                 *NewInterface
);
typedef EFI_STATUS (EFIAPI *EFI_UNINSTALL_PROTOCOL_INTERFACE) (
    IN EFI_HANDLE           Handle,
    IN EFI_GUID             *Protocol,
    IN VOID                 *Interface
);
typedef EFI_STATUS (EFIAPI *EFI_HANDLE_PROTOCOL) (
    IN EFI_HANDLE           Handle,
    IN EFI_GUID             *Protocol,
    OUT VOID                **Interface
);
typedef EFI_STATUS (EFIAPI *EFI_REGISTER_PROTOCOL_NOTIFY) (
    IN EFI_GUID             *Protocol,
    IN EFI_EVENT            Event,
    OUT VOID                **Registration
);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE) (
    IN EFI_LOCATE_SEARCH_TYPE   SearchType,
    IN EFI_GUID                 *Protocol OPTIONAL,
    IN VOID                     *SearchKey OPTIONAL,
    IN OUT UINTN                *BufferSize,
    OUT EFI_HANDLE              *Buffer
);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_DEVICE_PATH) (
    IN EFI_GUID                     *Protocol,
    IN OUT EFI_DEVICE_PATH_PROTOCOL **DevicePath,
    OUT EFI_HANDLE                  *Device
);
typedef EFI_STATUS (EFIAPI *EFI_INSTALL_CONFIGURATION_TABLE) (
    IN EFI_GUID                 *Guid,
    IN VOID                     *Table
);

typedef EFI_STATUS (EFIAPI *EFI_IMAGE_LOAD) (
    IN BOOLEAN                  BootPolicy,
    IN EFI_HANDLE               ParentHandle,
    IN EFI_DEVICE_PATH_PROTOCOL *DevicePath OPTIONAL,
    IN VOID                     *SourceBuffer OPTIONAL,
    IN UINTN                    SourceSize,
    OUT EFI_HANDLE              *ImageHandle
);
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_START) (
    IN EFI_HANDLE               ImageHandle,
    OUT UINTN                   *ExitDataSize,
    OUT CHAR16                  **ExitData OPTIONAL
);
typedef EFI_STATUS (EFIAPI *EFI_EXIT) (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_STATUS               ExitStatus,
    IN UINTN                    ExitDataSize,
    IN CHAR16                   *ExitData OPTIONAL
);
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_UNLOAD) (
    IN EFI_HANDLE               ImageHandle
);
typedef EFI_STATUS (EFIAPI *EFI_EXIT_BOOT_SERVICES) (
    IN EFI_HANDLE               ImageHandle,
    IN UINTN                    MapKey
);

typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_MONOTONIC_COUNT) (OUT UINT64 *Count);
typedef EFI_STATUS (EFIAPI *EFI_STALL) (IN UINTN Microseconds);
typedef EFI_STATUS (EFIAPI *EFI_SET_WATCHDOG_TIMER) (
    IN UINTN                    Timeout,
    IN UINT64                   WatchdogCode,
    IN UINTN                    DataSize,
    IN CHAR16                   *WatchdogData OPTIONAL
);

typedef EFI_STATUS (EFIAPI *EFI_CONNECT_CONTROLLER) (
    IN EFI_HANDLE               ControllerHandle,
    IN EFI_HANDLE               *DriverImageHandle OPTIONAL,
    IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath OPTIONAL,
    IN BOOLEAN                  Recursive
);
typedef EFI_STATUS (EFIAPI *EFI_DISCONNECT_CONTROLLER) (
    IN EFI_HANDLE               ControllerHandle,
    IN EFI_HANDLE               DriverImageHandle OPTIONAL,
    IN EFI_HANDLE               ChildHandle OPTIONAL
);

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL   0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL         0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL        0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER  0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER            0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE            0x00000020

typedef struct {
    EFI_HANDLE AgentHandle;
    EFI_HANDLE ControllerHandle;
    UINT32 Attributes;
    UINT32 OpenCount;
} EFI_OPEN_PROTOCOL_INFORMATION_ENTRY;

typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL) (
    IN EFI_HANDLE               Handle,
    IN EFI_GUID                 *Protocol,
    OUT VOID                    **Interface OPTIONAL,
    IN EFI_HANDLE               AgentHandle,
    IN EFI_HANDLE               ControllerHandle,
    IN UINT32                   Attributes
);
typedef EFI_STATUS (EFIAPI *EFI_CLOSE_PROTOCOL) (
    IN EFI_HANDLE               Handle,
    IN EFI_GUID                 *Protocol,
    IN EFI_HANDLE               AgentHandle,
    IN EFI_HANDLE               ControllerHandle
);
typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL_INFORMATION) (
    IN EFI_HANDLE                            Handle,
    IN EFI_GUID                             *Protocol,
    OUT EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer,
    OUT UINTN                               *EntryCount
);

typedef EFI_STATUS (EFIAPI *EFI_PROTOCOLS_PER_HANDLE) (
    IN EFI_HANDLE               Handle,
    OUT EFI_GUID                ***ProtocolBuffer,
    OUT UINTN                   *ProtocolBufferCount
);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE_BUFFER) (
    IN EFI_LOCATE_SEARCH_TYPE   SearchType,
    IN EFI_GUID                 *Protocol OPTIONAL,
    IN VOID                     *SearchKey OPTIONAL,
    OUT UINTN                   *NoHandle,
    OUT EFI_HANDLE              **Buffer
);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_PROTOCOL) (
    IN EFI_GUID                 *Protocol,
    IN VOID                     *Registration OPTIONAL,
    OUT VOID                    **Interface
);
typedef EFI_STATUS (EFIAPI *EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES) (IN OUT EFI_HANDLE *Handle, ...);
typedef EFI_STATUS (EFIAPI *EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES) (IN EFI_HANDLE Handle);

typedef EFI_STATUS (EFIAPI *EFI_CALCULATE_CRC32) (
    IN VOID     *Data,
    IN UINTN    DataSize,
    OUT UINT32  *Crc32
);

typedef VOID (EFIAPI *EFI_COPY_MEM) (IN VOID *Destination, IN VOID *Source, IN UINTN Length);
typedef VOID (EFIAPI *EFI_SET_MEM) (IN VOID *Buffer, IN UINTN Size, IN UINT8 Value);
typedef EFI_STATUS (EFIAPI *EFI_CREATE_EVENT_EX) (
    IN UINT32           Type,
    IN EFI_TPL          NotifyTpl,
    IN EFI_EVENT_NOTIFY NotifyFunction OPTIONAL,
    IN CONST VOID       *NotifyContext OPTIONAL,
    IN CONST EFI_GUID   *EventGroup OPTIONAL,
    OUT EFI_EVENT       *Event
);

typedef struct {
    EFI_TABLE_HEADER                            Hdr;

    EFI_RAISE_TPL                               RaiseTPL;
    EFI_RESTORE_TPL                             RestoreTPL;

    EFI_ALLOCATE_PAGES                          AllocatePages;
    EFI_FREE_PAGES                              FreePages;
    EFI_GET_MEMORY_MAP                          GetMemoryMap;
    EFI_ALLOCATE_POOL                           AllocatePool;
    EFI_FREE_POOL                               FreePool;

    EFI_CREATE_EVENT                            CreateEvent;
    EFI_SET_TIMER                               SetTimer;
    EFI_WAIT_FOR_EVENT                          WaitForEvent;
    EFI_SIGNAL_EVENT                            SignalEvent;
    EFI_CLOSE_EVENT                             CloseEvent;
    EFI_CHECK_EVENT                             CheckEvent;

    EFI_INSTALL_PROTOCOL_INTERFACE              InstallProtocolInterface;
    EFI_REINSTALL_PROTOCOL_INTERFACE            ReinstallProtocolInterface;
    EFI_UNINSTALL_PROTOCOL_INTERFACE            UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL                         HandleProtocol;
    VOID*                                       Reserved;
    EFI_REGISTER_PROTOCOL_NOTIFY                RegisterProtocolNotify;
    EFI_LOCATE_HANDLE                           LocateHandle;
    EFI_LOCATE_DEVICE_PATH                      LocateDevicePath;
    EFI_INSTALL_CONFIGURATION_TABLE             InstallConfigurationTable;

    EFI_IMAGE_LOAD                              LoadImage;
    EFI_IMAGE_START                             StartImage;
    EFI_EXIT                                    Exit;
    EFI_IMAGE_UNLOAD                            UnloadImage;
    EFI_EXIT_BOOT_SERVICES                      ExitBootServices;

    EFI_GET_NEXT_MONOTONIC_COUNT                GetNextMonotonicCount;
    EFI_STALL                                   Stall;
    EFI_SET_WATCHDOG_TIMER                      SetWatchdogTimer;

    EFI_CONNECT_CONTROLLER                      ConnectController;
    EFI_DISCONNECT_CONTROLLER                   DisconnectController;

    EFI_OPEN_PROTOCOL                           OpenProtocol;
    EFI_CLOSE_PROTOCOL                          CloseProtocol;
    EFI_OPEN_PROTOCOL_INFORMATION               OpenProtocolInformation;

    EFI_PROTOCOLS_PER_HANDLE                    ProtocolsPerHandle;
    EFI_LOCATE_HANDLE_BUFFER                    LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL                         LocateProtocol;
    EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES    InstallMultipleProtocolInterfaces;
    EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES  UninstallMultipleProtocolInterfaces;

    EFI_CALCULATE_CRC32                         CalculateCrc32;

    EFI_COPY_MEM                                CopyMem;
    EFI_SET_MEM                                 SetMem;
    EFI_CREATE_EVENT_EX                         CreateEventEx;
} EFI_BOOT_SERVICES;

typedef struct {
    EFI_GUID    VendorGuid;
    VOID        *VendorTable;
} EFI_CONFIGURATION_TABLE;

typedef struct {
    EFI_TABLE_HEADER                Hdr;
    CHAR16                          *FirmwareVendor;
    UINT32                          FirmwareRevision;
    EFI_HANDLE                      ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *ConIn;
    EFI_HANDLE                      ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                      StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    EFI_RUNTIME_SERVICES            *RuntimeServices;
    EFI_BOOT_SERVICES               *BootServices;
    UINTN                           NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE         *ConfigurationTable;
} EFI_SYSTEM_TABLE;

extern EFI_SYSTEM_TABLE* est;

typedef struct {
    UINT32                      Revision;
    EFI_HANDLE                  ParentHandle;
    EFI_SYSTEM_TABLE            *SystemTable;

    EFI_HANDLE                  DeviceHandle;
    VOID                        *FilePath;
    VOID                        *Reserved;

    UINT32                      LoadOptionsSize;
    VOID                        *LoadOptions;

    VOID                        *ImageBase;
    UINT64                      ImageSize;
    EFI_MEMORY_TYPE             ImageCodeType;
    EFI_MEMORY_TYPE             ImageDataType;
    EFI_IMAGE_UNLOAD            Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_IMAGE_UNLOAD) (IN EFI_HANDLE ImageHandle);

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME) (IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This, OUT EFI_FILE_PROTOCOL **Root);

struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64                                         Revision;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME    OpenVolume;
};

typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN) (IN EFI_FILE_PROTOCOL *This, OUT EFI_FILE_PROTOCOL **NewHandle, IN CHAR16 *FileName, IN UINT64 OpenMode, IN UINT64 Attributes);
typedef EFI_STATUS (EFIAPI *EFI_FILE_CLOSE) (IN EFI_FILE_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *EFI_FILE_DELETE) (IN EFI_FILE_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *EFI_FILE_READ) (IN EFI_FILE_PROTOCOL *This, IN OUT UINTN *BufferSize, OUT VOID *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_WRITE) (IN EFI_FILE_PROTOCOL *This, IN OUT UINTN *BufferSize, IN VOID *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_POSITION) (IN EFI_FILE_PROTOCOL *This, OUT UINT64 *Position);
typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_POSITION) (IN EFI_FILE_PROTOCOL *This, IN UINT64 Position);
typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_INFO) (IN EFI_FILE_PROTOCOL *This, IN EFI_GUID *InformationType, IN OUT UINTN *BufferSize, OUT VOID* Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_INFO) (IN EFI_FILE_PROTOCOL *This, IN EFI_GUID *InformationType, IN UINTN BufferSize, IN VOID* Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH) (IN EFI_FILE_PROTOCOL *This);

typedef struct {
    EFI_EVENT   Event;
    EFI_STATUS  Status;
    UINTN       BufferSize;
    VOID        *Buffer;
} EFI_FILE_IO_TOKEN;

typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN_EX) (IN EFI_FILE_PROTOCOL *This, OUT EFI_FILE_PROTOCOL **NewHandle, IN CHAR16 *FileName, IN UINT64 OpenMode, IN UINT64 Attributes, IN OUT EFI_FILE_IO_TOKEN *Token);
typedef EFI_STATUS (EFIAPI *EFI_FILE_READ_EX) (IN EFI_FILE_PROTOCOL *This, IN OUT EFI_FILE_IO_TOKEN *Token);
typedef EFI_STATUS (EFIAPI *EFI_FILE_WRITE_EX) (IN EFI_FILE_PROTOCOL *This, IN OUT EFI_FILE_IO_TOKEN *Token);
typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH_EX) (IN EFI_FILE_PROTOCOL *This, IN OUT EFI_FILE_IO_TOKEN *Token);

struct _EFI_FILE_PROTOCOL {
    UINT64                          Revision;
    EFI_FILE_OPEN                   Open;
    EFI_FILE_CLOSE                  Close;
    EFI_FILE_DELETE                 Delete;
    EFI_FILE_READ                   Read;
    EFI_FILE_WRITE                  Write;
    EFI_FILE_GET_POSITION           GetPosition;
    EFI_FILE_SET_POSITION           SetPosition;
    EFI_FILE_GET_INFO               GetInfo;
    EFI_FILE_SET_INFO               SetInfo;
    EFI_FILE_FLUSH                  Flush;
    EFI_FILE_OPEN_EX                OpenEx;     // Added for revision 2
    EFI_FILE_READ_EX                ReadEx;     // Added for revision 2
    EFI_FILE_WRITE_EX               WriteEx;    // Added for revision 2
    EFI_FILE_FLUSH_EX               FlushEx;    // Added for revision 2
};

typedef struct {
    UINT64      Size;
    UINT64      FileSize;
    UINT64      PhysicalSize;
    EFI_TIME    CreateTime;
    EFI_TIME    LastAccessTime;
    EFI_TIME    ModificationTime;
    UINT64      Attribute;
    CHAR16      *FileName;
} EFI_FILE_INFO;

typedef struct {
    UINT32  RedMask;
    UINT32  GreenMask;
    UINT32  BlueMask;
    UINT32  ReservedMask;
} EFI_PIXEL_BITMASK;

typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBltOnly,
    PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
    UINT32                      Version;
    UINT32                      HorizontalResolution;
    UINT32                      VerticalResolution;
    EFI_GRAPHICS_PIXEL_FORMAT   PixelFormat;
    EFI_PIXEL_BITMASK           PixelInformation;
    UINT32                      PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32                                  MaxMode;
    UINT32                                  Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION    *Info;
    UINTN                                   SizeOfInfo;
    EFI_PHYSICAL_ADDRESS                    FrameBufferBase;
    UINTN                                   FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct {
    UINT8   Blue;
    UINT8   Green;
    UINT8   Red;
    UINT8   Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

typedef enum {
    EfiBltVideoFill,
    EfiBltVideoToBltBuffer,
    EfiBltBufferToVideo,
    EfiBltVideoToVideo,
    EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE) (
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL             *This,
    IN UINT32                                   ModeNumber,
    OUT UINTN                                   *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION    **Info
);
typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE) (IN EFI_GRAPHICS_OUTPUT_PROTOCOL* This, IN UINT32 ModeNumber);
typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT) (
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL         *This,
    IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL    *BltBuffer OPTIONAL,
    IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION    BltOperation,
    IN UINTN                                SourceX,
    IN UINTN                                SourceY,
    IN UINTN                                DestinationX,
    IN UINTN                                DestinationY,
    IN UINTN                                Width,
    IN UINTN                                Height,
    IN UINTN                                Delta OPTIONAL
);

struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE     QueryMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE       SetMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT            Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE           *Mode;
};
