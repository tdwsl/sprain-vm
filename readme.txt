Sprain VM - tdwsl 2023

Sprain VM is a simple virtual processor for developing portable programs
with a small footprint.

It is 32-bit and uses little-endian for storing words. Its primary method of
interfacing is by using interrupts - the default interrupts are included in
the 'interrupt.inc' file. Programs are stored in a binary format, starting
with a 32-bit word to represent the base address for the program to be loaded
into. The stack expands positively, with the last pushed value sitting at
(sp-4).

Its assembly syntax is similar to x86, with a key difference being the use
of braces instead of square brackets for memory access. See the included
examples for use of the assembler.

Instruction Set:

     00              01              02              03
       04              05              06              07
         08              09              0A              0B
           0C              0D              0E              0F

00   brk             int I8          jmp I32         jr R8
       call I32        ret             mov ax,sp       mov sp,ax
         mov al,sr      mov sr,al        stc             clc
           --             --               --              --
10   mov ax,I32      mov bx,I32      mov cx,I32      mov dx,I32
       mov al,I8       mov bl,I8       mov cl,I8       mov dl,I8
         mov ax,(I32)    mov bx,(I32)    mov cx,(I32)    mov dx,(I32)
           mov al,(I32)    mov bl,(I32)    mov cl,(I32)    mov dl,(I32)
20   mov ax,(ax)     mov ax,(bx)     mov ax,(cx)     mov ax,(dx)
       mov bx,(ax)     mov bx,(bx)     mov bx,(cx)     mov bx,(dx)
         mov cx,(ax)     mov cx,(bx)     mov cx,(cx)     mov cx,(dx)
           mov dx,(ax)     mov dx,(bx)     mov dx,(cx)     mov dx,(dx)
30   mov al,(ax)     mov al,(bx)     mov al,(cx)     mov al,(dx)
       mov bl,(ax)     mov bl,(bx)     mov bl,(cx)     mov bl,(dx)
         mov cl,(ax)     mov cl,(bx)     mov cl,(cx)     mov cl,(dx)
           mov dl,(ax)     mov dl,(bx)     mov dl,(cx)     mov dl,(dx)
40   mov (ax),ax     mov (bx),ax     mov (cx),ax     mov (dx),ax
       mov (ax),bx     mov (bx),bx     mov (cx),bx     mov (dx),bx
         mov (ax),cx     mov (bx),cx     mov (cx),cx     mov (dx),cx
           mov (ax),dx     mov (bx),dx     mov (cx),dx     mov (dx),dx
50   mov (ax),al     mov (bx),al     mov (cx),al     mov (dx),al
       mov (ax),bl     mov (bx),bl     mov (cx),bl     mov (dx),bl
         mov (ax),cl     mov (bx),cl     mov (cx),cl     mov (dx),cl
           mov (ax),dl     mov (bx),dl     mov (cx),dl     mov (dx),dl
60   inc ax          inc bx          inc cx          inc dx
       inc al          inc bl          inc cl          inc dl
         dec ax          dec bx          dec cx          dec dx
           dec al          dec bl          dec cl          dec dl
70   inv ax          inv bx          inv cx          inv dx
       inv al          inv bl          inv cl          inv dl
         and ax,I32      and bx,I32      and cx,I32      and dx,I32
           and al,I8       and bl,I8       and cl,I8       and dl,I8
80   or ax,I32       or bx,I32       or cx,I32       or dx,I32
       or al,I8        or bl,I8        or cl,I8        or dl,I8
         xor ax,I32      xor bx,I32      xor cx,I32      xor dx,I32
           xor al,I8       xor bl,I8       xor cl,I8       xor dl,I8
90   shr ax,I8       shr bx,I8       shr cx,I8       shr dx,I8
       shl ax,I8       shl bx,I8       shl cx,I8       shl dx,I8
         add ax,I32      add bx,I32      add cx,I32      add dx,I32
           add al,I8       add bl,I8       add cl,I8       add dl,I8
A0   cmp ax,I32      cmp bx,I32      cmp cx,I32      cmp dx,I32
       cmp al,I8       cmp bl,I8       cmp cl,I8       cmp dl,I8
         push ax         push bx         push cx         push dx
           pop ax          pop bx          pop cx          pop dx
B0  jmp ax          jmp bx           jmp cx          jmp dx
      call ax         call bx          call cx         call dx
       jz R8           jc R8           jm R8           --
         jnz R8          jnc R8          jnm R8          --
C0   --              --              --              --
       --              --              --              --
         --              --              --              --
           --              --              --              --
D0   --              --              --              --
       --              --              --              --
         --              --              --              --
           --              --              --              --
E0   --              --              --              --
       --              --              --              --
         --              --              --              --
           --              --              --              --
F0   --              --              --              --
       --              --              --              --
         --              --              --              --
           --              --              --              (Ext)

Ext:

     00              01              02              03
       04              05              06              07
         08              09              0A              0B
           0C              0D              0E              0F

00   and ax,ax       and ax,bx       and ax,cx       and ax,dx
       and bx,ax       and bx,bx       and bx,cx       and bx,dx
         and cx,ax       and cx,bx       and cx,cx       and cx,dx
           and dx,ax       and dx,bx       and dx,cx       and dx,dx
10   and al,al       and al,bl       and al,cl       and al,dl
       and bl,al       and bl,bl       and bl,cl       and bl,dl
         and cl,al       and cl,bl       and cl,cl       and cl,dl
           and dl,al       and dl,bl       and dl,cl       and dl,dl
20   or ax,ax       or ax,bx       or ax,cx       or ax,dx
       or bx,ax       or bx,bx       or bx,cx       or bx,dx
         or cx,ax       or cx,bx       or cx,cx       or cx,dx
           or dx,ax       or dx,bx       or dx,cx       or dx,dx
30   or al,al       or al,bl       or al,cl       or al,dl
       or bl,al       or bl,bl       or bl,cl       or bl,dl
         or cl,al       or cl,bl       or cl,cl       or cl,dl
           or dl,al       or dl,bl       or dl,cl       or dl,dl
40   xor ax,ax       xor ax,bx       xor ax,cx       xor ax,dx
       xor bx,ax       xor bx,bx       xor bx,cx       xor bx,dx
         xor cx,ax       xor cx,bx       xor cx,cx       xor cx,dx
           xor dx,ax       xor dx,bx       xor dx,cx       xor dx,dx
50   xor al,al       xor al,bl       xor al,cl       xor al,dl
       xor bl,al       xor bl,bl       xor bl,cl       xor bl,dl
         xor cl,al       xor cl,bl       xor cl,cl       xor cl,dl
           xor dl,al       xor dl,bl       xor dl,cl       xor dl,dl
60   add ax,ax       add ax,bx       add ax,cx       add ax,dx
       add bx,ax       add bx,bx       add bx,cx       add bx,dx
         add cx,ax       add cx,bx       add cx,cx       add cx,dx
           add dx,ax       add dx,bx       add dx,cx       add dx,dx
70   add al,al       add al,bl       add al,cl       add al,dl
       add bl,al       add bl,bl       add bl,cl       add bl,dl
         add cl,al       add cl,bl       add cl,cl       add cl,dl
           add dl,al       add dl,bl       add dl,cl       add dl,dl
80   sub ax,ax       sub ax,bx       sub ax,cx       sub ax,dx
       sub bx,ax       sub bx,bx       sub bx,cx       sub bx,dx
         sub cx,ax       sub cx,bx       sub cx,cx       sub cx,dx
           sub dx,ax       sub dx,bx       sub dx,cx       sub dx,dx
90   sub al,al       sub al,bl       sub al,cl       sub al,dl
       sub bl,al       sub bl,bl       sub bl,cl       sub bl,dl
         sub cl,al       sub cl,bl       sub cl,cl       sub cl,dl
           sub dl,al       sub dl,bl       sub dl,cl       sub dl,dl
A0   cmp al,al       cmp al,bl       cmp al,cl       cmp al,dl
       cmp bl,al       cmp bl,bl       cmp bl,cl       cmp bl,dl
         cmp cl,al       cmp cl,bl       cmp cl,cl       cmp cl,dl
           cmp dl,al       cmp dl,bl       cmp dl,cl       cmp dl,dl
B0   --              --              --              --
       --              --              --              --
         --              --              --              --
           --              --              --              --
C0   --              --              --              --
       --              --              --              --
         --              --              --              --
           --              --              --              --
D0   --              --              --              --
       --              --              --              --
         --              --              --              --
           --              --              --              --
E0   --              --              --              --
       --              --              --              --
         --              --              --              --
           --              --              --              --
F0   --              --              --              --
       --              --              --              --
         --              --              --              --
           --              --              --              --

