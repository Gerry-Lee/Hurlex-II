/*
 * =====================================================================================
 *
 *       Filename:  idt.c
 *
 *    Description:  中断描述符表相关
 *
 *        Version:  1.0
 *        Created:  2014年11月04日 14时45分36秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Qianyi.lh (liuhuan), qianyi.lh@alibaba-inc.com
 *        Company:  Alibaba-Inc Aliyun
 *
 * =====================================================================================
 */

#include <debug.h>
#include <arch.h>

// 中断描述符表
idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__ ((aligned(16)));

// IDTR
idt_ptr_t idt_ptr;

// 中断处理函数的指针数组
interrupt_handler_t interrupt_handlers[INTERRUPT_MAX] __attribute__ ((aligned(4)));

// 设置中断描述符
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

// 声明加载 IDTR 的函数
extern void idt_flush(uint32_t);

// 中断处理函数指针类型
typedef void (*isr_irq_func_t)();

// 中断处理函数指针数组
static isr_irq_func_t isr_irq_func[INTERRUPT_MAX] = {
        [0]  = &isr0,  [1]  = &isr1,  [2]  = &isr2,  [3]  = &isr3,
        [4]  = &isr4,  [5]  = &isr5,  [6]  = &isr6,  [7]  = &isr7,
        [8]  = &isr8,  [9]  = &isr9,  [10] = &isr10, [11] = &isr11,
        [12] = &isr12, [13] = &isr13, [14] = &isr14, [15] = &isr15,
        [16] = &isr16, [17] = &isr17, [18] = &isr18, [19] = &isr19,
        [20] = &isr20, [21] = &isr21, [22] = &isr22, [23] = &isr23,
        [24] = &isr24, [25] = &isr25, [26] = &isr26, [27] = &isr27,
        [28] = &isr28, [29] = &isr29, [30] = &isr30, [31] = &isr31, 

        [32] = &irq0,  [33] = &irq1,  [34] = &irq2,  [35] = &irq3,
        [36] = &irq4,  [37] = &irq5,  [38] = &irq6,  [39] = &irq7,
        [40] = &irq8,  [41] = &irq9,  [42] = &irq10, [43] = &irq11,
        [44] = &irq12, [45] = &irq13, [46] = &irq14, [47] = &irq15, 
};

// 初始化中断描述符表
void init_idt(void)
{
        init_interrupt_chip();

        idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
        idt_ptr.base  = (uint32_t)&idt_entries;

        // 0~31:  用于 CPU 的中断处理
        // 32~47: Intel 保留
        int i;
        for (i = 0; i < 48; ++i) {
                idt_set_gate(i, (uint32_t)isr_irq_func[i], 0x08, 0x8E);
        }

        // 128 (0x80) 将来用于实现系统调用
        idt_set_gate(128, (uint32_t)isr128, 0x08, 0x8E);

        // 更新设置中断描述符表
        idt_flush((uint32_t)&idt_ptr);
}

// 设置中断描述符
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
        idt_entries[num].base_lo = base & 0xFFFF;
        idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

        idt_entries[num].sel = sel;
        idt_entries[num].always0 = 0;

        // 设置中断门的特权级别为 3
        idt_entries[num].flags = flags | 0x60;
}

// 调用中断处理函数
void isr_handler(pt_regs *regs)
{
        if (interrupt_handlers[regs->int_no]) {
              interrupt_handlers[regs->int_no](regs);
        } else {
                printk_color(rc_black, rc_blue, "Unhandled interrupt: %d\n", regs->int_no);
        }
}

// 注册一个中断处理函数
void register_interrupt_handler(uint8_t n, interrupt_handler_t h)
{
        interrupt_handlers[n] = h;
}

// IRQ 处理函数
void irq_handler(pt_regs *regs)
{
        // 重设PIC芯片
	clear_interrupt_chip(regs->int_no);

        if (interrupt_handlers[regs->int_no]) {
                interrupt_handlers[regs->int_no](regs);
        }
}
