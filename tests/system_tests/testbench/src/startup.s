.syntax	unified
.arch	armv7-m

	.section .stack
	.align	3
	.equ	Stack_Size, 0x00004000
	.globl	__StackTop
	.globl	__StackLimit
__StackLimit:
	.space	Stack_Size
	.size	__StackLimit, . - __StackLimit
__StackTop:
	.size	__StackTop, . - __StackTop

	.section .heap
	.align	3
	.equ	Heap_Size, 0x00001000
	.globl	__HeapBase
	.globl	__HeapLimit
__HeapBase:
	.if	Heap_Size
	.space	Heap_Size
	.endif
	.size	__HeapBase, . - __HeapBase
__HeapLimit:
	.size	__HeapLimit, . - __HeapLimit

.section .vectors
.align	2
.globl	__Vectors
__Vectors:
	.word	__StackTop
	.word	Reset_Handler
	.word	NMI_Handler
	.word	HardFault_Handler
	.word	MemManage_Handler
	.word	BusFault_Handler
	.word	UsageFault_Handler
	.word	0
	.word	0
	.word	0
	.word	0
	.word	SVC_Handler
	.word	DebugMon_Handler
	.word	0
	.word	PendSV_Handler
	.word	SysTick_Handler

.text               // Start of the code section (executable code).
.thumb              // Switch to Thumb instruction set (16-bit instructions).
.align  2           // Aligns the following code on a 4-byte boundary.

.thumb_func         // Marks the following function as a Thumb function.
.weak Reset_Handler
Reset_Handler:
	/* Reset handler implementation */
    bl _mainCRTStartup

.thumb_func         // Marks the following function as a Thumb function.
.weak NMI_Handler
NMI_Handler:
	/* NMI handler implementation */
    bx lr

.thumb_func         // Marks the following function as a Thumb function.
.weak HardFault_Handler
HardFault_Handler:
	/* HardFault handler implementation */
    bx lr

.thumb_func         // Marks the following function as a Thumb function.
.weak MemManage_Handler
MemManage_Handler:
	/* MemManage handler implementation */
    bx lr

.thumb_func         // Marks the following function as a Thumb function.
.weak BusFault_Handler
BusFault_Handler:
	/* BusFault handler implementation */
    bx lr

.thumb_func         // Marks the following function as a Thumb function.
.weak UsageFault_Handler
UsageFault_Handler:
	/* UsageFault handler implementation */
    bx lr

.thumb_func         // Marks the following function as a Thumb function.
.weak SVC_Handler
SVC_Handler:
	/* SVC handler implementation */
    bx lr

.thumb_func         // Marks the following function as a Thumb function.
.weak DebugMon_Handler
DebugMon_Handler:
	/* DebugMon handler implementation */
    bx lr

.thumb_func         // Marks the following function as a Thumb function.
.weak PendSV_Handler
PendSV_Handler:
	/* PendSV handler implementation */
    bx lr

.thumb_func         // Marks the following function as a Thumb function.
.weak SysTick_Handler
SysTick_Handler:
	/* SysTick handler implementation */
    bx lr
