#include <efi/efi_datatypes.h>
#include <efi/efi_misc.h>
#include <efi/efi.h>

#include <stdiok.h>

static const EFI_GUID EFI_LOADED_IMAGE_PROTOCOL_GUID = {
    .Data1 = 0x5B1B31A1,
    .Data2 = 0x9562,
    .Data3 = 0x11D2,
    .Data4 = { 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }
};

EFI_LOADED_IMAGE_PROTOCOL* getLoadedImageProtocol(EFI_HANDLE ImageHandle) {
    EFI_GUID** ImageProtocols = NULL;
    UINTN ProtoCount = 0;

    if (est->BootServices->ProtocolsPerHandle(ImageHandle, &ImageProtocols, &ProtoCount) != EFI_SUCCESS) {
        kputs(u"Error retrieving system loader image protocols\n\r");
        Terminate();
    }

    for (UINTN i = 0; i < ProtoCount; ++i) {
        if (kguidcmp(ImageProtocols[i], &EFI_LOADED_IMAGE_PROTOCOL_GUID)) {
            EFI_LOADED_IMAGE_PROTOCOL* efi_lip;

            if (est->BootServices->OpenProtocol(ImageHandle, ImageProtocols[i], (VOID**)&efi_lip, ImageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL) != EFI_SUCCESS) {
                kputs(u"Error retrieving system loader image information\n\r");
                Terminate();
            }

            est->BootServices->FreePool(ImageProtocols);
            return efi_lip;
        }
    }

    est->BootServices->FreePool(ImageProtocols);
    return NULL;
}

EFI_HANDLE getLoadedImageDevice(EFI_LOADED_IMAGE_PROTOCOL* efi_lip) {
    return efi_lip->DeviceHandle;
}
