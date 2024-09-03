#include <stdint.h>

#include <efi/efi_datatypes.h>
#include <efi/efi_misc.hpp>
#include <efi/efi.h>

#include <loader/basic_graphics.hpp>

#include <ldstdio.hpp>

namespace {
    static constexpr EFI_GUID EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID = {
        .Data1 = 0x9042A9DE,
        .Data2 = 0x23DC,
        .Data3 = 0x4A38,
        .Data4 = { 0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A }
    };

    // Preferred modes by descending priority (pairs of <width,height>), if no such mode is present, uses default mode.
    static constexpr UINT32 PreferredModes[6][2] = {
        { 1280,  720 },
        { 1024,  600 },
        {  800,  600 },
        { 2560, 1600 },
        { 1920, 1440 },
        { 1920, 1080 }
    };
}

BasicGraphics Loader::loadGraphics(void) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;
    if (EFI::sys->BootServices->LocateProtocol(
        const_cast<EFI_GUID*>(&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID),
        nullptr,
        reinterpret_cast<VOID**>(&gop)
    ) != EFI_SUCCESS) {
        Loader::puts(u"Could not find a suitable graphics output protocol\n\r");
        EFI::Terminate();
    }

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* gop_info = nullptr;
    UINTN gop_infosize = 0;
    UINTN gop_modesnumber = 0;

    if (gop->QueryMode(
        gop,
        gop->Mode == nullptr ? 0 : gop->Mode->Mode,
        &gop_infosize,
        &gop_info
    ) != EFI_SUCCESS) {
        Loader::puts(u"Error retrieving current video mode\n\r");
        EFI::Terminate();
    }

    EFI::sys->BootServices->FreePool(gop_info);

    gop_modesnumber = gop->Mode->MaxMode;

    UINT32 chosenMode = static_cast<UINT32>(-1);
    UINT32 chosenPriority = static_cast<UINT32>(-1);

    for (UINTN i = 0; i < gop_modesnumber; ++i) {
        if (gop->QueryMode(gop, i, &gop_infosize, &gop_info) != EFI_SUCCESS) {
            Loader::printf(u"Error retrieving video mode %d\n\r", i);
            continue;
        }

        for (size_t j = 0; j < sizeof(PreferredModes) / sizeof(UINT32[2]); ++j) {
            if (gop_info->HorizontalResolution == PreferredModes[j][0] &&
                gop_info->VerticalResolution == PreferredModes[j][1]
            ) {
                if (j < chosenPriority && gop_info->PixelFormat != PixelBltOnly) {
                    chosenMode = i;
                    chosenPriority = j;
                }
            }
        }

        EFI::sys->BootServices->FreePool(gop_info);
    }

    if (chosenMode != static_cast<UINT32>(-1)) {
        gop->SetMode(gop, chosenMode);
    }    

    BasicGraphics basicgfx = {
        .ResX = gop->Mode->Info->HorizontalResolution,
        .ResY = gop->Mode->Info->VerticalResolution,
        .PPSL = gop->Mode->Info->PixelsPerScanLine,
        .PXFMT = gop->Mode->Info->PixelFormat,
        .FBADDR = reinterpret_cast<uint32_t*>(gop->Mode->FrameBufferBase),
        .FBSIZE = gop->Mode->FrameBufferSize
    };

    return basicgfx;
}
