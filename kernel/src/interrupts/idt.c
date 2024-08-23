#include <stddef.h>
#include <stdint.h>

#define FLAGS_PRESENT       0x8000

#define FLAGS_DPL0          0x0000
#define FLAGS_DPL1          0x2000
#define FLAGS_DPL2          0x4000
#define FLAGS_DPL3          0x6000

#define FLAGS_EXCEPTION     0x0E00
#define FLAGS_TRAP          0x0F00

#define SS_CODE_KERNEL_GDT  0x0008
#define SS_CODE_KERNEL_LDT  void    // invalid
#define SS_CODE_USER_GDT    0x0018
#define SS_CODE_USER_LDT    void    // invalid

typedef struct {
    uint16_t offset0_15;
    uint16_t segment_selector;
    uint16_t flags;
    uint16_t offset16_31;
    uint32_t offset32_63;
    uint32_t reserved;
} IDTD;

typedef enum {
    DPL0,
    DPL1,
    DPL2,
    DPL3
} INTDPL;

typedef enum {
    EXCEPTION,
    TRAP
} INTTYPE;

extern uint8_t IDT[];

extern void int_divide_error(void);
extern void int_debug_trap(void);
extern void int_nmi_error(void);
extern void int_breakpoint_trap(void);
extern void int_overflow_trap(void);
extern void int_bound_error(void);
extern void int_invalidop_error(void);
extern void int_device_error(void);
extern void int_doublefault_error(void);
extern void int_coprocseg_error(void);
extern void int_invalidtss_error(void);
extern void int_segpresence_error(void);
extern void int_stack_error(void);
extern void int_gp_error(void);
extern void int_page_error(void);
extern void int_x87fp_error(void);
extern void int_align_error(void);
extern void int_machine_error(void);
extern void int_simd_error(void);
extern void int_virt_error(void);
extern void int_controlprotection_error(void);
extern void int_hypervirt_error(void);
extern void int_vmmcom_error(void);
extern void int_security_error(void);

static inline void register_core_interrupt(size_t intn, void(*_handler)(void), INTDPL dpl, INTTYPE type) {
    const uint64_t handler = (uint64_t)_handler;
    IDTD* descriptor = (IDTD*)IDT + intn;
    descriptor->offset0_15 = (uint16_t)handler;
    descriptor->segment_selector = SS_CODE_KERNEL_GDT;

    descriptor->flags = FLAGS_PRESENT;
    switch (dpl) {
        default:
        case DPL0:
            descriptor->flags |= FLAGS_DPL0;
            break;
        case DPL1:
            descriptor->flags |= FLAGS_DPL1;
            break;
        case DPL2:
            descriptor->flags |= FLAGS_DPL2;
            break;
        case DPL3:
            descriptor->flags |= FLAGS_DPL3;
            break;
    }
    switch (type) {
        default:
        case EXCEPTION:
            descriptor->flags |= FLAGS_EXCEPTION;
            break;
        case TRAP:
            descriptor->flags |= FLAGS_TRAP;
            break;
    }

    descriptor->offset16_31 = (uint16_t)(handler >> 16);
    descriptor->offset32_63 = (uint32_t)(handler >> 32);
}   


void kernel_idt_setup(void) {
    // setup each core exception handler
    register_core_interrupt(0,  &int_divide_error, DPL0, EXCEPTION);
    register_core_interrupt(1,  &int_debug_trap, 0, TRAP);
    register_core_interrupt(2,  &int_nmi_error, 0, EXCEPTION);
    register_core_interrupt(3,  &int_breakpoint_trap, 0, TRAP);
    register_core_interrupt(4,  &int_overflow_trap, 0, TRAP);
    register_core_interrupt(5,  &int_bound_error, 0, EXCEPTION);
    register_core_interrupt(6,  &int_invalidop_error, 0, EXCEPTION);
    register_core_interrupt(7,  &int_device_error, 0, EXCEPTION);
    register_core_interrupt(8,  &int_doublefault_error, 0, EXCEPTION);
    register_core_interrupt(9,  &int_coprocseg_error, 0, EXCEPTION);
    register_core_interrupt(10, &int_invalidtss_error, 0, EXCEPTION);
    register_core_interrupt(11, &int_segpresence_error, 0, EXCEPTION);
    register_core_interrupt(12, &int_stack_error, 0, EXCEPTION);
    register_core_interrupt(13, &int_gp_error, 0, EXCEPTION);
    register_core_interrupt(14, &int_page_error, 0, EXCEPTION);
    register_core_interrupt(16, &int_x87fp_error, 0, EXCEPTION);
    register_core_interrupt(17, &int_align_error, 0, EXCEPTION);
    register_core_interrupt(18, &int_machine_error, 0, EXCEPTION);
    register_core_interrupt(19, &int_simd_error, 0, EXCEPTION);
    register_core_interrupt(20, &int_virt_error, 0, EXCEPTION);
    register_core_interrupt(21, &int_controlprotection_error, 0, EXCEPTION);
    register_core_interrupt(28, &int_hypervirt_error, 0, EXCEPTION);
    register_core_interrupt(29, &int_vmmcom_error, 0, EXCEPTION);
    register_core_interrupt(30, &int_security_error, 0, EXCEPTION);
}

void register_irq(unsigned int irq_line, void(*handler)(void), unsigned is_trap) {
    register_core_interrupt(0x20 + irq_line, handler, DPL0, is_trap ? TRAP : EXCEPTION);
}
