reset:
ldr sp, =4000

.cpu arm926ej-s
.fpu softvfp
.text

.section .rodata
.global	UART0DR
.global main
.global reset


mov r0, #65
ldr r2, =UART0DR
ldr	r2, [r2]
str	r0, [r2]
b .

@ UART0 out
UART0DR: @ 32
	.word 270471168
