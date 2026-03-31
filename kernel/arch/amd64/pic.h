#pragma once

/*
 * 8259 Programmable Interrupt Controller (PIC)
 *
 * It manages hardware interrupts by sending them in order to the CPU interrupt
 * lines. This is a legacy device, as modern systems use the APIC to handle
 * multicore, but it it still emulated bt the BIOS for backward compatibility.
 *
 * IRQ 0 to 7 are handled by the Master PIC while 8-15 by the Slave PIC in
 * cascade mode.
 *
 * https://wiki.osdev.org/8259_PIC
 */

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

#define EOI 0x20

/* Initializes the 8259 PIC device for protected mode, with offset remappings. */
void pic_init (void);

/* Enable (clear) or disable (set) the given irq line. */
void pic_edit_mask (int irq, int set);

/* Sends the End of Interrupt command. Must be called at the end of every ISR. */
void pic_eoi (int irq);
