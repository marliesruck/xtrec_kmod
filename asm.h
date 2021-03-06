/** @file x86/asm.h
 *  @brief x86-specific assembly functions
 *  @author matthewj S2008
 */

#ifndef X86_ASM_H
#define X86_ASM_H

/** @brief Read address of IDT */
void *idt_base(void);

/** @brief Disable interrupts */
void disable_interrupts(void);

/** @brief Disable interrupts */
void enable_interrupts(void);

#endif /* !X86_ASM_H */
