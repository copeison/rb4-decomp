; IDA disassembly evidence for the common render-texture lifecycle.
; Functions: 0x69B6E0-0x69B79C

; render_texture_construct at 0x69b6e0
0069B6E0  push    rbp; Constructs the exact 168-byte common texture state and applies its default sampler/resource values.
0069B6E1  mov     rbp, rsp
0069B6E4  push    rbx
0069B6E5  push    rax
0069B6E6  mov     rbx, rdi
0069B6E9  call    sub_6427D0
0069B6EE  lea     rax, unk_1935678
0069B6F5  vxorps  ymm0, ymm0, ymm0
0069B6F9  mov     ecx, 0FFFFFFFFh
0069B6FE  add     rax, 10h
0069B702  mov     [rbx], rax
0069B705  mov     dword ptr [rbx+10h], 0FFFFFFFFh
0069B70C  vmovups ymmword ptr [rbx+4Ch], ymm0
0069B711  vmovups ymmword ptr [rbx+34h], ymm0
0069B716  vmovups ymmword ptr [rbx+14h], ymm0
0069B71B  vmovd   xmm0, ecx
0069B71F  vmovdqu xmmword ptr [rbx+6Ch], xmm0
0069B724  mov     qword ptr [rbx+80h], 0
0069B72F  mov     dword ptr [rbx+88h], 0
0069B739  mov     byte ptr [rbx+8Ch], 0
0069B740  mov     dword ptr [rbx+90h], 0FFFFFFFFh
0069B74A  mov     dword ptr [rbx+94h], 0
0069B754  mov     qword ptr [rbx+98h], 0
0069B75F  mov     dword ptr [rbx+0A0h], 0FFFFFFFFh
0069B769  add     rsp, 8
0069B76D  pop     rbx
0069B76E  pop     rbp
0069B76F  retn

; render_texture_destruct at 0x69b770
0069B770  jmp     nullsub_49; Empty common render-texture destructor.

; render_texture_delete at 0x69b780
0069B780  push    rbp; Deleting common render-texture destructor.
0069B781  mov     rbp, rsp
0069B784  push    rbx
0069B785  push    rax
0069B786  mov     rbx, rdi
0069B789  call    nullsub_49
0069B78E  mov     rdi, rbx
0069B791  add     rsp, 8
0069B795  pop     rbx
0069B796  pop     rbp
0069B797  jmp     sub_37BF50
