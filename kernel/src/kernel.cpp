#include <cstddef>
#include <cstdint>

#include <efi.h>

#include <devices/PS2/controller.hpp>
#include <devices/PS2/Keyboard.hpp>

#include <interrupts/idt.hpp>
#include <interrupts/pic.hpp>
#include <interrupts/pit.hpp>
#include <interrupts/KernelPanic.hpp>
#include <interrupts/SystemTimer.hpp>

#include <mm/gdt.hpp>
#include <mm/PhysicalMemory.hpp>
#include <mm/VirtualMemory.hpp>
#include <mm/VirtualMemoryLayout.hpp>

#include <multitasking/Task.hpp>
#include <multitasking/KernelTask.hpp>

#include <screen/Log.hpp>

extern uint8_t kernel_init_array_start[];
extern uint8_t kernel_init_array_end[];

namespace {
    static const EFI_RUNTIME_SERVICES* rtServices;

    static void _kernel_ctx_init() {
        const size_t initializer_count = (kernel_init_array_end - kernel_init_array_start) / sizeof(void(*)(void));

        for (size_t i = 0; i < initializer_count; ++i) {
            (*reinterpret_cast<void(**)(void)>(kernel_init_array_start + i * sizeof(void(*)(void))))();
        }

        const uint64_t mmap_size = *reinterpret_cast<uint64_t*>(VirtualMemoryLayout::OS_BOOT_DATA + VirtualMemoryLayout::BOOT_MEMORY_MAP_SIZE_OFFSET);
        rtServices = reinterpret_cast<EFI_RUNTIME_SERVICES*>(VirtualMemoryLayout::OS_BOOT_DATA + mmap_size + VirtualMemoryLayout::BOOT_RUNTIME_SERVICES_ADDRESS);
    }

    static inline void SetupPhysicalMemory() {
        auto status = PhysicalMemory::Setup();
        if (status != PhysicalMemory::StatusCode::SUCCESS) {
            if (status == PhysicalMemory::StatusCode::OUT_OF_MEMORY) {
                Panic::PanicShutdown(rtServices, "PMM INITIALIZATION FAILED (OUT OF MEMORY)\n\r");
            }
            else {
                Panic::PanicShutdown(rtServices, "PMM INITIALIZATION FAILED (UNKNOWN REASON)\n\r");
            }
        }
    }

    static inline void SetupVirtualMemory() {
        auto status = VirtualMemory::Setup();
        if (status != VirtualMemory::StatusCode::SUCCESS) {
            Panic::PanicShutdown(rtServices, "VMM INITIALIZATION FAILED\n\r");
        }
    }

    static inline void SetupPS2Keyboard() {
        uint32_t status = 0;

        if ((status = Devices::PS2::initialize_ps2_controller()) != 0) {
            Log::puts("PS/2 Controller initialization failed.\n\r");
            Interrupts::PIC::mask_irq(1);
            return;
        }

        Log::puts("PS/2 Controller Initialized.\n\r");

        if ((status = Devices::PS2::identify_ps2_port_1()) > 0xFFFF) {
            Log::puts("PS/2 Identify failed for device on port 1.\n\r");
            Interrupts::PIC::mask_irq(1);
            return;
        }

        auto statusCode = Devices::PS2::initializeKeyboard();

        if (statusCode != Devices::PS2::StatusCode::SUCCESS) {
            Log::puts("PS/2 keyboard initialization failed.\n\r");
            Log::puts("No PS/2 input will be provided unless a USB keyboard is connected.\n\r");
            Interrupts::PIC::mask_irq(1);
            return;
        }

        Log::puts("PS/2 Keyboard Initialized.\n\r");
        Interrupts::register_irq(1, &Devices::PS2::PS2_IRQ1_handler, 0);
    }
}

__attribute__((section(".userembedded"))) void idleTask(void) {
    while (1);
}

__attribute__((section(".userembedded"))) void idleTask2(void) {
    while (1);
}

extern "C" int kmain() {
    __asm__ volatile("cli");

    _kernel_ctx_init();

    VirtualMemory::kernel_gdt_setup();
    Interrupts::kernel_idt_load();
    Interrupts::kernel_idt_setup();

    Log::Setup();
    Log::puts("Log Terminal Enabled\n\r");

    SetupPhysicalMemory();
    Log::puts("PMM Initialized\n\r");

    SetupVirtualMemory();
    Log::puts("VMM Initialized\n\r");

    Interrupts::register_irq(0, &Interrupts::SystemTimer::PIT_IRQ0_handler, 0);
    Interrupts::PIC::initialize_pic();
    Interrupts::PIT::initialize_pit();

    SetupPS2Keyboard();

    auto status = Multitasking::loadKernelTask(reinterpret_cast<void*>(&idleTask));
    if (status != Multitasking::StatusCode::SUCCESS) {
        Panic::Panic("Failed to load kernel task\n\r");
    }

    status = Multitasking::loadKernelTask(reinterpret_cast<void*>(&idleTask2));
    if (status != Multitasking::StatusCode::SUCCESS) {
        Panic::Panic("Failed to load kernel task\n\r");
    }

    __asm__ volatile("sti");

    while (1);
}
