.text
.globl teste0s
teste0s:
   push   %rbp
   mov    %rsp,%rbp
   sub    $0x10,%rsp
   movl   $0x0,-0x10(%rbp)
   addl   $0x1,-0x10(%rbp)
   mov    -0x10(%rbp),%eax
   leaveq
   retq
