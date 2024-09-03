#pragma once

namespace Interrupts {
	namespace Core {
		extern "C" {
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
		}
	}
}