BITS 64
global _main
   extern _printf
   extern _putchar
section .text
_main:
   sub rsp, 4
   mov dword[rsp+4], 65
   sub rsp, 4
   mov dword[rsp+4], 1
   sub rsp, 4
   mov dword[rsp+4], 1
   sub rsp, 4
   mov ebx, dword[rsp+12]
   mov ecx, dword[rsp+8]
   add rbx, rcx
   mov edx, dword[rsp+16]
   add rdx, rbx
   mov dword[rsp+4], edx
   xor rax, rax
   mov eax, dword[rsp+4]
   push rax
   call _putchar
   pop  rax
   xor rax, rax
section .data
   intformt: db "%d",0
   chrformt: db "%c",0
