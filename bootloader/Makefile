MAIN = loader_main.cpp
MAIN_INIT = pre_setup.asm

MAIN_OBJECT = loader_main.o
MAIN_INIT_OBJECT = pre_setup.o

TARGET = BOOTX64.EFI
SRCDIR = src
OBJECTSDIR = objects
LIBSDIR = lib

CCNOLINK = x86_64-w64-mingw32-gcc 	\
	-mabi=ms

CXXNOLINK = x86_64-w64-mingw32-g++ 	\
	-mabi=ms

CC = $(CCNOLINK) 					\
	-Wl,-subsystem,10 				\
	-Wl,-e,efi_main

CXX = $(CXXNOLINK)					\
	-Wl,-subsystem,10				\
	-Wl,-e,efi_main

AR = ar 							\
	-rcs

CFLAGS = 							\
	-std=c23 						\
	-Wall 							\
	-Wextra 						\
	-Wpedantic 						\
	-fms-extensions 				\
	-mno-red-zone 					\
	-ffreestanding 					\
	-nostdlib 						\
	-Iinclude

CXXFLAGS =							\
	-std=c++23						\
	-Wall							\
	-Wextra							\
	-Wpedantic						\
	-fms-extensions					\
	-mno-red-zone					\
	-ffreestanding					\
	-nostdlib						\
	-Iinclude

all: $(TARGET)

$(OBJECTSDIR):
	@mkdir -p $(OBJECTSDIR)/ldstdio
	@mkdir -p $(OBJECTSDIR)/ldstdlib
	@mkdir -p $(OBJECTSDIR)/efi
	@mkdir -p $(OBJECTSDIR)/loader

$(LIBSDIR):
	@mkdir -p $(LIBSDIR)

ldstdio_sources=$(wildcard $(SRCDIR)/ldstdio/*.cpp)
ldstdio_objects=$(patsubst $(SRCDIR)/ldstdio/%.cpp,$(OBJECTSDIR)/ldstdio/%.o,$(ldstdio_sources))

$(ldstdio_objects): $(OBJECTSDIR)/ldstdio/%.o: $(SRCDIR)/ldstdio/%.cpp | $(OBJECTSDIR)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) -c $< -o $@

ldstdlib_sources=$(wildcard $(SRCDIR)/ldstdlib/*.cpp)
ldstdlib_objects=$(patsubst $(SRCDIR)/ldstdlib/%.cpp,$(OBJECTSDIR)/ldstdlib/%.o,$(ldstdlib_sources))

$(ldstdlib_objects): $(OBJECTSDIR)/ldstdlib/%.o: $(SRCDIR)/ldstdlib/%.cpp | $(OBJECTSDIR)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) -c $< -o $@

$(LIBSDIR)/ldlibc.lib: $(ldstdio_objects) $(ldstdlib_objects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^

efilib_sources=$(wildcard $(SRCDIR)/efi/*.cpp)
efilib_objects=$(patsubst $(SRCDIR)/efi/%.cpp,$(OBJECTSDIR)/efi/%.o,$(efilib_sources))

$(efilib_objects): $(OBJECTSDIR)/efi/%.o: $(SRCDIR)/efi/%.cpp | $(OBJECTSDIR)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) -c $< -o $@

$(LIBSDIR)/efi.lib: $(efilib_objects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^

loaderlib_sources=$(wildcard $(SRCDIR)/loader/*.cpp)
loaderlib_objects=$(patsubst $(SRCDIR)/loader/%.cpp,$(OBJECTSDIR)/loader/%.o,$(loaderlib_sources))

$(loaderlib_objects): $(OBJECTSDIR)/loader/%.o: $(SRCDIR)/loader/%.cpp | $(OBJECTSDIR)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) -c $< -o $@

$(LIBSDIR)/loader.lib: $(loaderlib_objects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^

$(OBJECTSDIR)/$(MAIN_INIT_OBJECT): $(SRCDIR)/$(MAIN_INIT) | $(OBJECTSDIR)
	@echo Building $@
	@nasm -fwin64 $< -o $@

$(OBJECTSDIR)/$(MAIN_OBJECT): $(SRCDIR)/$(MAIN) | $(OBJECTSDIR)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTSDIR)/$(MAIN_OBJECT) $(OBJECTSDIR)/$(MAIN_INIT_OBJECT) $(LIBSDIR)/ldlibc.lib $(LIBSDIR)/efi.lib $(LIBSDIR)/loader.lib
	@echo Creating $@
	@$(CXX) $(CXXFLAGS) -s -L$(LIBSDIR) -Wl,--start-group -l:efi.lib -l:ldlibc.lib -l:loader.lib -o $@ $(OBJECTSDIR)/$(MAIN_INIT_OBJECT) $< -Wl,--end-group
	@strip -R .idata -R .pdata -R .xdata $(TARGET)

.PHONY: clean
clean:
	@rm -rf lib
	@rm -rf objects
	@rm -f BOOTX64.EFI
	@echo Done.
