#pragma once

#ifndef __EFI_STANDALONE__

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
