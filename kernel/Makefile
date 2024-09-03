
EFI_HEADER_LOC = ../bootloader/include/efi

OBJECTSDIR = objects
LIBSDIR = lib

CCNOLINK = gcc		                    \
	-mabi=ms

CXXNOLINK = g++ 	                    \
	-mabi=ms                			

CCKER = $(CCNOLINK)                     \
    -Wl,-e,kmain                        \
    -Wl,-T,linker.ld					\
	-Wl,--build-id=none

CXXKER = $(CXXNOLINK)                   \
    -Wl,-e,kmain                        \
    -Wl,-T,linker.ld					\
	-Wl,--build-id=none					\
	-z noexecstack

AR = ar                                 \
    -rcs

CFLAGS = 								\
	-std=c2x							\
	-Wall								\
	-Wextra								\
	-Wpedantic 							\
	-fms-extensions 					\
	-mno-red-zone 						\
	-ffreestanding 						\
	-nostdlib 							\
	-I$(EFI_HEADER_LOC) 				\
	-Iinclude							\
	-D__EFI_STANDALONE__                \
	-fno-PIE							\
	-mcmodel=large						\
	-static								\
	-fno-exceptions						

CXXFLAGS =							    \
	-std=c++23						    \
	-Wall							    \
	-Wextra							    \
	-Wpedantic						    \
	-fms-extensions					    \
	-mno-red-zone					    \
	-ffreestanding					    \
	-nostdlib						    \
    -I$(EFI_HEADER_LOC)                 \
	-Iinclude                           \
    -D__EFI_STANDALONE__				\
	-fno-PIE							\
	-mcmodel=large						\
	-static								\
	-fno-exceptions						\
	-fno-rtti

CFASTNOSSE = 							\
	-Ofast								\
	-mno-sse

all: kernel.img
$(OBJECTSDIR):
	@mkdir $(OBJECTSDIR)
$(LIBSDIR):
	@mkdir $(LIBSDIR)
devices_cxxsources = src/devices/PS2/Keyboard.cpp src/devices/PS2/KeyboardEvent.cpp src/devices/PS2/Keypoints.cpp 
devices_cxxobjects = $(patsubst src/devices/%.cpp, objects/devices/%.o,$(devices_cxxsources))
$(devices_cxxobjects): objects/devices/%.o: src/devices/%.cpp | $(OBJECTSDIR)
	@mkdir -p $(@D)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) $(CFASTNOSSE) -o $@ -c $<
devices_asmsources = src/devices/PS2/controller.asm src/devices/PS2/keyboard_handler_irq.asm 
devices_asmobjects = $(patsubst src/devices/%.asm, objects/devices/%.o,$(devices_asmsources))
$(devices_asmobjects): objects/devices/%.o: src/devices/%.asm | $(OBJECTSDIR)
	@mkdir -p $(@D)
	@echo Building $@
	@nasm -felf64 $< -o $@
lib/devices.lib: $(devices_cxxobjects) $(devices_cobjects) $(devices_asmobjects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^
interrupts_cxxsources = src/interrupts/CoreDump.cpp src/interrupts/idt.cpp src/interrupts/KernelPanic.cpp src/interrupts/SystemTimer.cpp 
interrupts_cxxobjects = $(patsubst src/interrupts/%.cpp, objects/interrupts/%.o,$(interrupts_cxxsources))
$(interrupts_cxxobjects): objects/interrupts/%.o: src/interrupts/%.cpp | $(OBJECTSDIR)
	@mkdir -p $(@D)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) $(CFASTNOSSE) -o $@ -c $<
interrupts_asmsources = src/interrupts/CoreDumpSetup.asm src/interrupts/gpr_isr_saving.asm src/interrupts/idt_load.asm src/interrupts/pic.asm src/interrupts/pit.asm src/interrupts/system_timer_irq.asm src/interrupts/core/align_error.asm src/interrupts/core/bound_error.asm src/interrupts/core/breakpoint_trap.asm src/interrupts/core/controlprotection_error.asm src/interrupts/core/coprocoseg_error.asm src/interrupts/core/debug_trap.asm src/interrupts/core/device_error.asm src/interrupts/core/dfault_abort.asm src/interrupts/core/divide_error.asm src/interrupts/core/gp_error.asm src/interrupts/core/hypervirt_error.asm src/interrupts/core/invalidop_error.asm src/interrupts/core/invalidtss_error.asm src/interrupts/core/machine_error.asm src/interrupts/core/nmi_error.asm src/interrupts/core/overflow_trap.asm src/interrupts/core/page_error.asm src/interrupts/core/security_error.asm src/interrupts/core/segpresence_error.asm src/interrupts/core/simd_error.asm src/interrupts/core/stack_error.asm src/interrupts/core/virt_error.asm src/interrupts/core/vmmcom_error.asm src/interrupts/core/x87fp_error.asm 
interrupts_asmobjects = $(patsubst src/interrupts/%.asm, objects/interrupts/%.o,$(interrupts_asmsources))
$(interrupts_asmobjects): objects/interrupts/%.o: src/interrupts/%.asm | $(OBJECTSDIR)
	@mkdir -p $(@D)
	@echo Building $@
	@nasm -felf64 $< -o $@
lib/interrupts.lib: $(interrupts_cxxobjects) $(interrupts_cobjects) $(interrupts_asmobjects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^
mm_cxxsources = src/mm/Heap.cpp src/mm/PhysicalMemory.cpp src/mm/VirtualMemory.cpp 
mm_cxxobjects = $(patsubst src/mm/%.cpp, objects/mm/%.o,$(mm_cxxsources))
$(mm_cxxobjects): objects/mm/%.o: src/mm/%.cpp | $(OBJECTSDIR)
	@mkdir -p $(@D)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) $(CFASTNOSSE) -o $@ -c $<
mm_asmsources = src/mm/gdt.asm 
mm_asmobjects = $(patsubst src/mm/%.asm, objects/mm/%.o,$(mm_asmsources))
$(mm_asmobjects): objects/mm/%.o: src/mm/%.asm | $(OBJECTSDIR)
	@mkdir -p $(@D)
	@echo Building $@
	@nasm -felf64 $< -o $@
lib/mm.lib: $(mm_cxxobjects) $(mm_cobjects) $(mm_asmobjects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^
screen_cxxsources = src/screen/Log.cpp 
screen_cxxobjects = $(patsubst src/screen/%.cpp, objects/screen/%.o,$(screen_cxxsources))
$(screen_cxxobjects): objects/screen/%.o: src/screen/%.cpp | $(OBJECTSDIR)
	@mkdir -p $(@D)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) $(CFASTNOSSE) -o $@ -c $<
lib/screen.lib: $(screen_cxxobjects) $(screen_cobjects) $(screen_asmobjects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^
_cxx_sources = src/kernel.cpp 
_cxx_objects = $(patsubst src/%.cpp,objects/%.o,$(_cxx_sources))
$(_cxx_objects): objects/%.o: src/%.cpp | $(OBJECTSDIR)
	@echo Building $@
	@$(CXXNOLINK) $(CXXFLAGS) $(CFASTNOSSE) -o $@ -c $<
kernel.img: $(_cxx_objects) lib/devices.lib lib/interrupts.lib lib/mm.lib lib/screen.lib 
	@echo Building $@
	@$(CXXKER) $(CXXFLAGS) -Wl,--start-group -Llib -l:devices.lib -l:interrupts.lib -l:mm.lib -l:screen.lib -o $@ $< -Wl,--end-group
.PHONY: clean
clean:
	@rm -rf lib
	@rm -rf objects
	@rm -f kernel.img
	@echo Done.