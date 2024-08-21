#include <stdint.h>

#include <efi/efi_datatypes.h>
#include <efi/efi_misc.h>
#include <efi/efi.h>

#include <loader/basic_graphics.h>

#include <stdiok.h>

static const EFI_GUID EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID = {
    .Data1 = 0x9042A9DE,
    .Data2 = 0x23DC,
    .Data3 = 0x4A38,
    .Data4 = { 0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A }
};

// Preferred modes by descending priority (pairs of <width,height>), if no such mode is present, uses default mode.
static const UINT32 PreferredModes[6][2] = {
    { 1280,  720 },
    { 1024,  600 },
    {  800,  600 },
    { 2560, 1600 },
    { 1920, 1440 },
    { 1920, 1080 }
};

BasicGraphics loadGraphics() {
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    if (est->BootServices->LocateProtocol((EFI_GUID*)&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, NULL, (void**)&gop) != EFI_SUCCESS) {
        kputs(u"Could not find a suitable graphics output protocol\n\r");
        Terminate();
    }

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* gop_info;
    UINTN gop_infosize;
    UINTN gop_modesnumber;

    if (gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &gop_infosize, &gop_info) != EFI_SUCCESS) {
        kputs(u"Error retrieving current video mode\n\r");
        Terminate();
    }

    est->BootServices->FreePool(gop_info);

    gop_modesnumber = gop->Mode->MaxMode;

    UINT32 chosenMode = (UINT32)-1;
    UINT32 chosenPriority = (UINT32)-1;

    for (UINTN i = 0; i < gop_modesnumber; ++i) {
        if (gop->QueryMode(gop, i, &gop_infosize, &gop_info) != EFI_SUCCESS) {
            kprintf(u"Error retrieving video mode %d\n\r", i);
            Terminate();
        }

        for (size_t j = 0; j < sizeof(PreferredModes) / sizeof(UINT32[2]); ++j) {
            if (gop_info->HorizontalResolution == PreferredModes[j][0] && gop_info->VerticalResolution == PreferredModes[j][1]) {
                if (j < chosenPriority && gop_info->PixelFormat != PixelBltOnly) {                
                    chosenMode = i;
                    chosenPriority = j;
                }
            }
        } 

        est->BootServices->FreePool(gop_info);
    }

    if (chosenMode != (UINT32)-1) {
        gop->SetMode(gop, chosenMode);
    }

    BasicGraphics basicgfx;

    basicgfx.ResX = gop->Mode->Info->HorizontalResolution,
    basicgfx.ResY = gop->Mode->Info->VerticalResolution,
    basicgfx.PPSL = gop->Mode->Info->PixelsPerScanLine;
    basicgfx.PXFMT = gop->Mode->Info->PixelFormat;
    basicgfx.FBADDR = (uint32_t*)gop->Mode->FrameBufferBase;
    basicgfx.FBSIZE = gop->Mode->FrameBufferSize;

    return basicgfx;
}
