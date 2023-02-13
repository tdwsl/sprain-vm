Sprain VM - tdwsl 2022

Sprain VM is a simple virtual processor that is easy to use and easy to
implement. Its purpose is to develop portable programs with a small footprint.

It is 32-bits, little-endian and the stack expands positively, with the last
pushed value sitting at rsp-4. Its primary method of interfacing is through
interrupts.

The VM has 16 registers:

  zero, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, rsp, rpc

Instruction Set:

As well as the instructions below, the assembler supports some pseudo-ops:

  jmp I32  -> mov rsp,I32
  bra I8   -> add rsp,I8
  ret      -> pop rsp

Register-register instructions use the next byte to store the register.
The sizes of instructions according to their operands are:

  R      -> 1 byte
  I8     -> 2 bytes
  R,R    -> 2 bytes
  R,I8   -> 2 bytes
  I32    -> 5 bytes
  R,I32  -> 5 bytes

     00              01              02              03
       04              05              06              07
         08              09              0A              0B
           0C              0D              0E              0F

00   int I8          call I32        beq R,R I8      bne R,R I8
       mov R,(R)       mov (R),R       mol R,(R)       mol (R),R
         mov R,R         and R,R         or R,R          xor R,R
           add R,R         sub R,R         --              --
10   --              mov r1,I32      mov r2,I32      mov r3,I32
       mov r4,I32      mov r5,I32      mov r6,I32      mov r7,I32
         mov r8,I32      mov r9,I32      mov r10,I32     mov r11,I32
           mov r12,I32     mov r13,I32     mov rsp,I32     mov rpc,I32
20   --              add r1,I8       add r2,I8       add r3,I8
       add r4,I8       add r5,I8       add r6,I8       add r7,I8
         add r8,I8       add r9,I8       add r10,I8      add r11,I8
           add r12,I8      add r13,I8      add rsp,I8      add rpc,I8
30   push zero       push r1         push r2         push r3
       push r4         push r5         push r6         push r7
         push r8         push r9         push r10        push r11
           push r12        push r13        push rsp        push rpc
40   --              pop r1          pop r2          pop r3
       pop r4          pop r5          pop r6          pop r7
         pop r8          pop r9          pop r10         pop r11
           pop r12         pop r13         pop rsp         pop rpc
50   --              inv r1          inv r2          inv r3
       inv r4          inv r5          inv r6          inv r7
         inv r8          inv r9          inv r10         inv r11
           inv r12         inv r13         inv rsp         inv rpc
60   --              shr r1,I8       shr r2,I8       shr r3,I8
       shr r4,I8       shr r5,I8       shr r6,I8       shr r7,I8
         shr r8,I8       shr r9,I8       shr r10,I8      shr r11,I8
           shr r12,I8      shr r13,I8      shr rsp,I8      shr rpc,I8
70   --              shl r1,I8       shl r2,I8       shl r3,I8
       shl r4,I8       shl r5,I8       shl r6,I8       shl r7,I8
         shl r8,I8       shl r9,I8       shl r10,I8      shl r11,I8
           shl r12,I8      shl r13,I8      shl rsp,I8      shl rpc,I8

