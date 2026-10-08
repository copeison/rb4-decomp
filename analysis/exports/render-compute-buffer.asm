; IDA disassembly evidence for the common render compute-buffer lifecycle.
; Functions: 0x636C70-0x636DB6

; render_create_compute_buffer at 0x636c70
00636C70  push    rbp; Creates and initializes a compute buffer through the active render backend. Descriptive inferred name.
00636C71  mov     rbp, rsp
00636C74  push    rbx
00636C75  push    rax
00636C76  lea     rcx, g_render_system
00636C7D  mov     rax, rdi
00636C80  mov     rsi, rax
00636C83  mov     rcx, [rcx]
00636C86  mov     rdi, [rcx+130h]
00636C8D  mov     rcx, [rdi]
00636C90  call    qword ptr [rcx+68h]
00636C93  mov     rbx, rax
00636C96  mov     rdi, [rbx+10h]
00636C9A  imul    rdi, [rbx+18h]
00636C9F  call    sub_37BF60
00636CA4  mov     [rbx+40h], rax
00636CA8  mov     rdi, rbx
00636CAB  mov     rax, [rbx]
00636CAE  call    qword ptr [rax+50h]
00636CB1  mov     rax, rbx
00636CB4  add     rsp, 8
00636CB8  pop     rbx
00636CB9  pop     rbp
00636CBA  retn

; render_compute_buffer_construct at 0x636cc0
00636CC0  push    rbp
00636CC1  mov     rbp, rsp
00636CC4  push    r14
00636CC6  push    rbx
00636CC7  mov     r14, rsi
00636CCA  mov     rbx, rdi
00636CCD  call    sub_6427D0
00636CD2  lea     rax, unk_192ED98
00636CD9  add     rax, 10h
00636CDD  mov     [rbx], rax
00636CE0  mov     qword ptr [rbx+40h], 0
00636CE8  vmovups ymm0, ymmword ptr [r14]
00636CED  vmovups ymm1, ymmword ptr [r14+10h]
00636CF3  vmovups ymmword ptr [rbx+20h], ymm1
00636CF8  vmovups ymmword ptr [rbx+10h], ymm0
00636CFD  pop     rbx
00636CFE  pop     r14
00636D00  pop     rbp
00636D01  retn

; render_compute_buffer_destruct at 0x636d10
00636D10  push    rbp
00636D11  mov     rbp, rsp
00636D14  push    rbx
00636D15  push    rax
00636D16  lea     rax, unk_192ED98
00636D1D  mov     rbx, rdi
00636D20  add     rax, 10h
00636D24  mov     [rbx], rax
00636D27  mov     rdi, [rbx+40h]
00636D2B  test    rdi, rdi
00636D2E  jz      short loc_636D35
00636D30  call    sub_37BF70
00636D35  mov     rdi, rbx
00636D38  add     rsp, 8
00636D3C  pop     rbx
00636D3D  pop     rbp
00636D3E  jmp     nullsub_49

; render_compute_buffer_delete at 0x636d50
00636D50  push    rbp
00636D51  mov     rbp, rsp
00636D54  push    rbx
00636D55  push    rax
00636D56  lea     rax, unk_192ED98
00636D5D  mov     rbx, rdi
00636D60  add     rax, 10h
00636D64  mov     [rbx], rax
00636D67  mov     rdi, [rbx+40h]
00636D6B  test    rdi, rdi
00636D6E  jz      short loc_636D75
00636D70  call    sub_37BF70
00636D75  mov     rdi, rbx
00636D78  call    nullsub_49
00636D7D  mov     rdi, rbx
00636D80  add     rsp, 8
00636D84  pop     rbx
00636D85  pop     rbp
00636D86  jmp     sub_37BF50

; render_compute_buffer_type at 0x636db0
00636DB0  mov     eax, 0FFFFFFFFh
00636DB5  retn
