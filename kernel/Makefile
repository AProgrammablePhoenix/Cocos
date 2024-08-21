
EFI_HEADER_LOC = ../bootloader/include/efi

OBJECTSDIR = objects
LIBSDIR = lib

CCNOLINK = clang 						\
	-target x86_64-unknown-windows

AR = ar -cr

CCKER = $(CCNOLINK) 					\
	-fuse-ld=lld 						\
	-Wl,-subsystem:efi_application 		\
	-Wl,-entry:kmain 					\
	-Wl,-FIXED,-BASE:0xffff800000000000

CFLAGS = 								\
	-std=c23							\
	-Wall								\
	-Wextra								\
	-Wpedantic 							\
	-fms-extensions 					\
	-Wno-microsoft-anon-tag 			\
	-mno-red-zone 						\
	-ffreestanding 						\
	-nostdlib 							\
	-I$(EFI_HEADER_LOC) 				\
	-Iinclude							\
	-D__EFI_STANDALONE__

CFASTNOSSE = 							\
	-Ofast								\
	-mno-sse

all: kernel.exe
$(OBJECTSDIR):
	@mkdir $(OBJECTSDIR)
$(LIBSDIR):
	@mkdir $(LIBSDIR)
interrupts_csources = src/interrupts/core_dump.c src/interrupts/idt.c src/interrupts/kpanic.c src/interrupts/system_timer.c 
interrupts_cobjects = $(patsubst src/interrupts/%.c, objects/interrupts/%.o,$(interrupts_csources))
$(interrupts_cobjects): objects/interrupts/%.o: src/interrupts/%.c | $(OBJECTSDIR)
	@powershell -Command "mkdir -Force $(@D)" 1> nul
	@echo Building $@
	@$(CCNOLINK) $(CFLAGS) $(CFASTNOSSE) -o $@ -c $<
interrupts_asmsources = src/interrupts/core_dump_setup.asm src/interrupts/gpr_isr_saving.asm src/interrupts/i8042.asm src/interrupts/i8254.asm src/interrupts/i8259A.asm src/interrupts/kernel_idt.asm src/interrupts/system_timer_irq.asm src/interrupts/core/align_error.asm src/interrupts/core/bound_error.asm src/interrupts/core/breakpoint_trap.asm src/interrupts/core/controlprotection_error.asm src/interrupts/core/coprocoseg_error.asm src/interrupts/core/debug_trap.asm src/interrupts/core/device_error.asm src/interrupts/core/dfault_abort.asm src/interrupts/core/divide_error.asm src/interrupts/core/gp_error.asm src/interrupts/core/hypervirt_error.asm src/interrupts/core/invalidop_error.asm src/interrupts/core/invalidtss_error.asm src/interrupts/core/machine_error.asm src/interrupts/core/nmi_error.asm src/interrupts/core/overflow_trap.asm src/interrupts/core/page_error.asm src/interrupts/core/security_error.asm src/interrupts/core/segpresence_error.asm src/interrupts/core/simd_error.asm src/interrupts/core/stack_error.asm src/interrupts/core/virt_error.asm src/interrupts/core/vmmcom_error.asm src/interrupts/core/x87fp_error.asm 
interrupts_asmobjects = $(patsubst src/interrupts/%.asm, objects/interrupts/%.o,$(interrupts_asmsources))
$(interrupts_asmobjects): objects/interrupts/%.o: src/interrupts/%.asm | $(OBJECTSDIR)
	@powershell -Command "mkdir -Force $(@D)" 1> nul
	@echo Building $@
	@nasm -fwin64 $< -o $@
lib/interrupts.lib: $(interrupts_cobjects) $(interrupts_asmobjects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^
mm_csources = src/mm/pmm.c src/mm/vmm.c 
mm_cobjects = $(patsubst src/mm/%.c, objects/mm/%.o,$(mm_csources))
$(mm_cobjects): objects/mm/%.o: src/mm/%.c | $(OBJECTSDIR)
	@powershell -Command "mkdir -Force $(@D)" 1> nul
	@echo Building $@
	@$(CCNOLINK) $(CFLAGS) $(CFASTNOSSE) -o $@ -c $<
mm_asmsources = src/mm/kernel_gdt.asm 
mm_asmobjects = $(patsubst src/mm/%.asm, objects/mm/%.o,$(mm_asmsources))
$(mm_asmobjects): objects/mm/%.o: src/mm/%.asm | $(OBJECTSDIR)
	@powershell -Command "mkdir -Force $(@D)" 1> nul
	@echo Building $@
	@nasm -fwin64 $< -o $@
lib/mm.lib: $(mm_cobjects) $(mm_asmobjects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^
screen_csources = src/screen/tty.c 
screen_cobjects = $(patsubst src/screen/%.c, objects/screen/%.o,$(screen_csources))
$(screen_cobjects): objects/screen/%.o: src/screen/%.c | $(OBJECTSDIR)
	@powershell -Command "mkdir -Force $(@D)" 1> nul
	@echo Building $@
	@$(CCNOLINK) $(CFLAGS) $(CFASTNOSSE) -o $@ -c $<
lib/screen.lib: $(screen_cobjects) $(screen_asmobjects) | $(LIBSDIR)
	@echo Creating $@
	@$(AR) $@ $^
kernel.exe: src/kernel.c lib/interrupts.lib lib/mm.lib lib/screen.lib 
	@echo Building $@
	@$(CCKER) $(CFLAGS) -Llib -linterrupts -lmm -lscreen -o $@ $<
.PHONY: clean
clean:
	if exist lib rmdir /Q /S lib
	if exist objects rmdir /Q /S objects
	if exist kernel.exe del /Q /S kernel.exe
