.PHONY: bootloader kernel
all: bootloader kernel

bootloader:
	@echo Building $@
	@$(MAKE) --no-print-directory -C $@

kernel:
	@echo Building $@
	@$(MAKE) --no-print-directory -C $@

copyToDisk: bootloader kernel
	@xcopy  /v /f /y bootloader\BOOTX64.EFI X:\EFI\BOOT\BOOTX64.EFI | find /V "File(s) copied"
	@xcopy /v /f /y kernel\kernel.exe X:\EFI\BOOT\kernel.exe | find /V "File(s) copied"


.PHONY: clean
clean:
	@$(MAKE) --no-print-directory -C bootloader clean
	@$(MAKE) --no-print-directory -C kernel clean