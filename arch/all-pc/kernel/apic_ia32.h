#ifndef APIC_IA32_H
#define APIC_IA32_H
/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: IA-32 APIC hardware definitions.
    Lang: english
*/

#include <aros/cpu.h>

#include "i8259a.h"

// From CPU and LAPIC point of view we have 256 interrupt vectors. The first 32 are reserved
// for CPU exceptions. Further, there are 16 vectors reserved for legacy XT-PIC (which can be
// eventually remapped to LAPIC with help of IOAPIC). Official APIC IRQ base starts right
// after legacy XT-PIC
#define APIC_IRQ_MAX            256
#define APIC_IRQ_BASE           (X86_CPU_EXCEPT_COUNT + I8259A_IRQCOUNT)

// Local APIC exceptions, with SysCall being the last (int $0xff)! The numeric values start
// at X86_CPU_EXCEPT_COUNT in order to make the handler simplier
enum
{
    APIC_EXCEPT_HEARTBEAT = X86_CPU_EXCEPT_COUNT,
    APIC_EXCEPT_IPI_NOP,
    APIC_EXCEPT_IPI_IPI_STOP,
    APIC_EXCEPT_IPI_RESUME,
    APIC_EXCEPT_IPI_RESCHEDULE,
    APIC_EXCEPT_IPI_CALL_HOOK,
    APIC_EXCEPT_IPI_CAUSE,
    APIC_EXCEPT_ERROR,
    APIC_EXCEPT_SYSCALL,
    APIC_EXCEPT_SPURIOUS,
    APIC_EXCEPT_TOP
};

#define APIC_CPU_EXCEPT_COUNT   (APIC_EXCEPT_TOP - X86_CPU_EXCEPT_COUNT)
#define APIC_CPU_EXCEPT_BASE    (APIC_IRQ_MAX - APIC_CPU_EXCEPT_COUNT)
#define APIC_IRQ_COUNT          (APIC_CPU_EXCEPT_BASE - APIC_IRQ_BASE)

#define APIC_CPU_EXCEPT_TO_VECTOR(num)  ((num) - X86_CPU_EXCEPT_COUNT + APIC_CPU_EXCEPT_BASE)
#define GET_EXCEPTION_NUMBER(irq) \
    ((irq) < X86_CPU_EXCEPT_COUNT ? (irq) : ((irq) - APIC_CPU_EXCEPT_BASE + X86_CPU_EXCEPT_COUNT))
#define GET_DEVICE_IRQ(irq)     ((irq) - X86_CPU_EXCEPT_COUNT)
#define IS_APIC_EXCEPTION(irq)  (((irq) >= APIC_CPU_EXCEPT_BASE) && ((irq) < APIC_EXCEPT_SYSCALL))
#define IS_EXCEPTION(irq)       ((irq) < X86_CPU_EXCEPT_COUNT || (irq) >= APIC_CPU_EXCEPT_BASE)


/* Local APIC base address register (MSR #27) */
#define MSR_LAPIC_BASE          0x1B

#define APIC_BOOTSTRAP          (1 << 8)
#define APIC_ENABLE             (1 << 11)

/* APIC hardware registers */

#define APIC_ID		        0x20
#define APIC_VERSION	        0x30
#define APIC_TPR	        0x80	                /* Task Priority Register			*/
#define APIC_APR	        0x90	                /* Arbitration Priority Register		*/
#define APIC_PPR	        0xA0	                /* Processor Priority Register			*/
#define APIC_EOI	        0xB0	                /* End Of Interrupt Register			*/
#define APIC_REMOTE_READ        0xC0
#define APIC_LDR	        0xD0	                /* Logical Destination Register			*/
#define APIC_DFR	        0xE0	                /* Destination Format Register			*/
#define APIC_SVR	        0xF0	                /* Spurious Interrupt Vector Register		*/
#define APIC_ISR	        0x100	                /* In Service Register				*/
#define APIC_TMR	        0x180	                /* Trigger Mode Register			*/
#define APIC_IRR	        0x200	                /* Interrupt Request Register			*/
#define APIC_ESR	        0x280	                /* Error Status Register			*/
#define APIC_ICRL	        0x300	                /* Interrupt Command Register low part		*/
#define APIC_ICRH	        0x310	                /* Interrupt Command Register high part 	*/
#define APIC_TIMER_VEC	        0x320	                /* Timer local vector table entry		*/
#define APIC_THERMAL_VEC        0x330	                /* Thermal local vector table entry		*/
#define APIC_PCOUNT_VEC	        0x340	                /* Performance counter local vector table entry	*/
#define APIC_LINT0_VEC	        0x350	                /* Local interrupt 0 vector table entry		*/
#define APIC_LINT1_VEC	        0x360	                /* Local interrupt 1 vector table entry		*/
#define APIC_ERROR_VEC	        0x370	                /* Error vector table entry			*/
#define APIC_TIMER_ICR	        0x380	                /* Timer initial count				*/
#define APIC_TIMER_CCR	        0x390	                /* Timer current count				*/
#define APIC_TIMER_DIV	        0x3E0	                /* Timer divide configuration register		*/

/* ID shift value */
#define APIC_ID_SHIFT           24

/* Version register */
#define APIC_VERSION_MASK       0x000000FF	        /* The actual version number			*/
#define APIC_LVT_SHIFT	        16	                /* No. of entries in LVT, less one              */
#define APIC_LVT_MASK 	        (0xFF << APIC_LVT_SHIFT)
#define APICB_EAS	        31	                /* Whether this APIC has extended address space	*/
#define APICF_EAS	        (1 << APICB_EAS)
#define APIC_EAS	        APICF_EAS

/* Macros to help parsing version */
#define APIC_INTEGRATED(ver)    (ver & 0x000000F0)
#define APIC_LVT(ver)	        ((ver & APIC_LVT_MASK) >> APIC_LVT_SHIFT)

/* LDR shift value */
#define LDR_ID_SHIFT 24

/* Destination format (interrupt model) */
#define DFR_CLUSTER             (0x0 << 28)
#define DFR_FLAT                (0xF << 28)

#define SVR_VEC_MASK            0xFF
#define SVR_ASE	                (1 << 8)                /* APIC enable                                  */
#define SVR_FCC	                (1 << 9)                /* focus disabled                               */
#define SVR_DEOI                (1 << 12)               /* directed EOI (broadcast suppression)         */

/* Error register */
#define ERR_SAE                 (1 << 2)                /* Sent accept error                            */
#define ERR_RAE                 (1 << 3)                /* Receive accept error                         */
#define ERR_SIV                 (1 << 5)                /* Sent illegal vector                          */
#define ERR_RIV                 (1 << 6)                /* Received illegal vector                      */
#define ERR_IRA                 (1 << 7)                /* Illegal register address                     */

/* ICRL register */
#define ICR_VEC_MASK	        0x000000FF	        /* Vector number (request argument) mask	*/
#define ICRB_DM                 8
#define ICR_DM_LOWP             (0x01 << ICRB_DM)	/* Lowest Priority			        */
#define ICR_DM_SMI              (0x02 << ICRB_DM)	/* SMI			                        */
#define ICR_DM_NMI              (0x04 << ICRB_DM)	/* NMI			                        */
#define ICR_DM_INIT             (0x05 << ICRB_DM)	/* INIT request (reset the CPU)			*/
#define ICR_DM_STARTUP          (0x06 << ICRB_DM)	/* STARTUP request (run from specified address)	*/
#define ICRB_DS		        12	                /* Delivery status flag				*/
#define ICRF_DS		        (1 << ICRB_DS)
#define ICR_DS		        ICRF_DS
#define ICRB_DSH                18                      /* Destination shorthand                        */
#define ICR_DSH_SELF            (0x01 << ICRB_DSH)
#define ICR_DSH_ALL             (0x02 << ICRB_DSH)
#define ICR_DSH_ALLBUTSELF      (0x03 << ICRB_DSH)


#define ICRB_INT_LEVEL          14	                /* Assert (set) or deassert (reset)		*/
#define ICRF_INT_LEVEL          (1 << ICRB_INT_LEVEL)
#define ICR_INT_ASSERT          ICRF_INT_LEVEL
#define ICRB_INT_TRIGM          15
#define ICRF_INT_TRIGM          (1 << ICRB_INT_TRIGM)
#define ICR_INT_LEVELTRIG       ICRF_INT_TRIGM

/* Local vector table entry fields */
#define LVT_VEC_MASK            0x0000FF		/* Vector no				        */
#define LVT_MT_MASK             0x000700		/* Message type				        */
#define LVT_MT_FIXED            0x000000
#define LVT_MT_SMI              0x000200
#define LVT_MT_NMI              0x000400
#define LVT_MT_EXT              0x000700
#define LVT_DS	                0x001000		/* Delivery status bit 			        */
#define LVT_ACTIVE_LOW          0x002000		/* Polarity flag (1 = low active)	        */
#define LVT_RIR	                0x004000		/* Remote IRR				        */
#define LVT_TGM_LEVEL           0x008000		/* Level-trigger mode			        */
#define LVT_MASK                0x010000		/* Mask bit				        */
#define LVT_TMM_PERIOD          0x020000		/* Periodic timer mode			        */

/* Timer divisors */
#define TIMER_DIV_1             0x0B
#define TIMER_DIV_2             0x00
#define TIMER_DIV_4             0x01
#define TIMER_DIV_8             0x02
#define TIMER_DIV_16            0x03
#define TIMER_DIV_32            0x08
#define TIMER_DIV_64            0x09
#define TIMER_DIV_128           0x0A

/*
 * MS HyperV CPUID definitions
 */

// TODO: Move to public header

#define HYPERV_CPUID_MIN			0x40000005
#define HYPERV_CPUID_MAX			0x4000ffff
 
#define HYPERV_CPUID_MAXANDVENDOR		0x40000000
#define HYPERV_CPUID_INTERFACE			0x40000001
#define HYPERV_CPUID_VERSION			0x40000002
#define HYPERV_CPUID_FEATURES			0x40000003
#define HYPERV_CPUID_ENLIGHT_INFO		0x40000004
#define HYPERV_CPUID_IMPLM_LIMIT		0x40000005

/*
 * MS HyperV MSR definitions
 */
#define MSR_HYPERV_TSC_FREQUENCY		0x40000022		/* TSC frequency */
#define MAS_HYPERV_APIC_FREQUENCY		0x40000023		/* APIC timer frequency */

/* Register access macro to make the code more readable */
#define APIC_REG(base, reg) *((volatile ULONG *)(base + reg))

#endif /* !APIC_IA32_H */
