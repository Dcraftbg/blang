BITS 64
global _main
   extern _printf
   extern _putchar
section .text
_main:
   push 72
   call _putchar
   pop rax
   push 101
   call _putchar
   pop rax
   push 108
   call _putchar
   pop rax
   push 108
   call _putchar
   pop rax
   push 111
   call _putchar
   pop rax
   push 10
   call _putchar
   pop rax
   push 87
   call _putchar
   pop rax
   push 111
   call _putchar
   pop rax
   push 114
   call _putchar
   pop rax
   push 108
   call _putchar
   pop rax
   push 100
   call _putchar
   pop rax
   push 33
   call _putchar
   pop rax
   xor rax, rax
section .data
   intformt: db "%d",0
   chrformt: db "%c",0