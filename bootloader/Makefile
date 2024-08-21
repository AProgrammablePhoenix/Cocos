SOURCE = loader_main.c
SOURCE_INIT = pre_setup.asm
TARGET = BOOTX64.EFI
TARGET_INIT = pre_setup.o

SRCDIR = src
OBJECTSDIR = objects
LIBSDIR = lib

CCNOLINK = clang \
	-target x86_64-unknown-windows

CC = $(CCNOLINK) \
	-fuse-ld=lld \
	-Wl,-subsystem:efi_application \
	-Wl,-entry:efi_main

AR = ar \
	-cr

CFLAGS = \
	-std=c23 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-fms-extensions \
	-Wno-microsoft-anon-tag \
	-mno-red-zone \
	-ffreestanding \
	-nostdlib \
	-Iinclude


all: $(TARGET)

$(OBJECTSDIR):
	@mkdir $(OBJECTSDIR)\stdiok
	@mkdir $(OBJECTSDIR)\stdlibk
	@mkdir $(OBJECTSDIR)\efi
	@mkdir $(OBJECTSDIR)\loader

$(LIBSDIR):
	@mkdir $(LIBSDIR)

stdiok_sources=$(wildcard $(SRCDIR)/stdiok/*.c)
stdiok_objects=$(patsubst $(SRCDIR)/stdiok/%.c,$(OBJECTSDIR)/stdiok/%.o,$(stdiok_sources))

$(stdiok_objects): $(OBJECTSDIR)/stdiok/%.o: $(SRCDIR)/stdiok/%.c | $(OBJECTSDIR)
	@echo Building $^
	@$(CCNOLINK) $(CFLAGS) -c $^ -o $@

stdlibk_sources=$(wildcard $(SRCDIR)/stdlibk/*.c)
stdlibk_objects=$(patsubst $(SRCDIR)/stdlibk/%.c,$(OBJECTSDIR)/stdlibk/%.o,$(stdlibk_sources))

$(stdlibk_objects): $(OBJECTSDIR)/stdlibk/%.o: $(SRCDIR)/stdlibk/%.c | $(OBJECTSDIR)
	@echo Building $^
	@$(CCNOLINK) $(CFLAGS) -c $^ -o $@	

$(LIBSDIR)/libck.lib: $(stdlibk_objects) $(stdiok_objects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^

efilib_sources=$(wildcard $(SRCDIR)/efi/*.c)
efilib_objects=$(patsubst $(SRCDIR)/efi/%.c,$(OBJECTSDIR)/efi/%.o,$(efilib_sources))

$(efilib_objects): $(OBJECTSDIR)/efi/%.o: $(SRCDIR)/efi/%.c | $(OBJECTSDIR)
	@echo Building $^
	@$(CCNOLINK) $(CFLAGS) -c $^ -o $@

$(LIBSDIR)/efi.lib: $(efilib_objects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^

loaderlib_sources=$(wildcard $(SRCDIR)/loader/*.c)
loaderlib_objects=$(patsubst $(SRCDIR)/loader/%.c,$(OBJECTSDIR)/loader/%.o,$(loaderlib_sources))

$(loaderlib_objects): $(OBJECTSDIR)/loader/%.o: $(SRCDIR)/loader/%.c | $(OBJECTSDIR)
	@echo Building $^
	@$(CCNOLINK) $(CFLAGS) -c $^ -o $@

$(LIBSDIR)/loader.lib: $(loaderlib_objects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^

$(OBJECTSDIR)/$(TARGET_INIT): $(SRCDIR)/$(SOURCE_INIT) | $(OBJECTSDIR)
	@echo Building $<
	@nasm -fwin64 $< -o $@

$(TARGET): $(SRCDIR)/$(SOURCE) $(OBJECTSDIR)/$(TARGET_INIT) $(LIBSDIR)/libck.lib $(LIBSDIR)/efi.lib $(LIBSDIR)/loader.lib
	@echo Building $<
	@$(CC) $(CFLAGS) -L$(LIBSDIR) -llibck -lefi -lloader -o $@ $(OBJECTSDIR)/$(TARGET_INIT) $<

.PHONY: clean
clean:
	if exist lib rmdir /Q /S lib
	if exist objects rmdir /Q /S objects
	if exist BOOTX64.EFI del BOOTX64.EFI