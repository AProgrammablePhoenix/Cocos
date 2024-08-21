#include <stdint.h>

#include <efi/efi_datatypes.h>
#include <efi/efi_misc.h>
#include <efi/efi_image_services.h>
#include <efi/efi_fs.h>
#include <efi/efi.h>

#include <stdiok.h>

#include <loader/paging_loader.h>
#include <loader/tty_font.h>

#define FONT_FILE_PATH u"\\EFI\\BOOT\\tty_font.psf\0"

const void* loadFont(EFI_HANDLE ImageHandle, PML4E* pml4, const PagingInformation* PI) {
    EFI_LOADED_IMAGE_PROTOCOL* efi_lip = getLoadedImageProtocol(ImageHandle);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* efi_sfsp = getDeviceSFSP(ImageHandle, efi_lip->DeviceHandle);
    EFI_FILE_PROTOCOL* efi_root_fsp = openDeviceVolume(efi_sfsp);
    EFI_FILE_PROTOCOL* efi_font_fsp = openReadOnlyFile(efi_root_fsp, FONT_FILE_PATH);

    if (efi_font_fsp == NULL) {
        kputs(u"TTY font file was either not found, or no suitable protocol was found to locate/open it\n\r");
        Terminate();
    }

    EFI_FILE_INFO* FontInfo = getFileInfo(efi_font_fsp);
    UINT64 FontSize = FontInfo->FileSize;
    est->BootServices->FreePool(FontInfo);

    size_t requiredPages = (FontSize + PAGE_SIZE - 1) /  PAGE_SIZE;

    void* FontBuffer = NULL;
    est->BootServices->AllocatePages(AllocateAnyPages, LoaderPersistentMemory, requiredPages, (EFI_PHYSICAL_ADDRESS*)&FontBuffer);

    if (efi_font_fsp->Read(efi_font_fsp, &FontSize, FontBuffer) != EFI_SUCCESS) {
        kputs(u"Error reading font file\n\r");
        Terminate();
    }
    efi_font_fsp->Close(efi_font_fsp);

    remapTTYFont(pml4, (const void**)&FontBuffer, FontSize, PI);

    return FontBuffer;
}
