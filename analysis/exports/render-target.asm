; IDA disassembly evidence for the common render-target lifecycle.
; Functions: 0x11B2CD0-0x11B2E09

; render_target_construct at 0x11b2cd0
011B2CD0  push    rbp; Constructs the 32-byte common render-target state and optionally creates its owned 1552-byte target state.
011B2CD1  mov     rbp, rsp
011B2CD4  push    r15
011B2CD6  push    r14
011B2CD8  push    rbx
011B2CD9  push    rax
011B2CDA  mov     r15d, edx
011B2CDD  mov     r14d, esi
011B2CE0  mov     rbx, rdi
011B2CE3  call    sub_4486F0
011B2CE8  lea     rax, unk_19A5B18
011B2CEF  vxorps  xmm0, xmm0, xmm0
011B2CF3  add     rax, 10h
011B2CF7  test    r15b, r15b
011B2CFA  mov     [rbx], rax
011B2CFD  mov     [rbx+0Ch], r15b
011B2D01  vmovups xmmword ptr [rbx+10h], xmm0
011B2D06  jz      short loc_11B2D2A
011B2D08  mov     edi, 610h
011B2D0D  call    sub_37BF40
011B2D12  mov     r15, rax
011B2D15  xor     edx, edx
011B2D17  mov     esi, r14d
011B2D1A  mov     rdi, r15
011B2D1D  call    sub_6B40A0
011B2D22  mov     [rbx+10h], r15
011B2D26  mov     [rbx+18h], r15
011B2D2A  add     rsp, 8
011B2D2E  pop     rbx
011B2D2F  pop     r14
011B2D31  pop     r15
011B2D33  pop     rbp
011B2D34  retn

; render_target_destruct at 0x11b2d40
011B2D40  push    rbp; Deletes owned render-target state and clears both target-state pointers.
011B2D41  mov     rbp, rsp
011B2D44  push    r14
011B2D46  push    rbx
011B2D47  lea     rax, unk_19A5B18
011B2D4E  mov     rbx, rdi
011B2D51  add     rax, 10h
011B2D55  mov     [rbx], rax
011B2D58  cmp     byte ptr [rbx+0Ch], 0
011B2D5C  jz      short loc_11B2D7A
011B2D5E  mov     rdi, [rbx+10h]
011B2D62  lea     r14, [rbx+10h]
011B2D66  test    rdi, rdi
011B2D69  jz      short loc_11B2D71
011B2D6B  mov     rax, [rdi]
011B2D6E  call    qword ptr [rax+8]
011B2D71  vxorps  xmm0, xmm0, xmm0
011B2D75  vmovups xmmword ptr [r14], xmm0
011B2D7A  mov     rdi, rbx
011B2D7D  pop     rbx
011B2D7E  pop     r14
011B2D80  pop     rbp
011B2D81  jmp     nullsub_43

; render_target_delete at 0x11b2d90
011B2D90  push    rbp; Deleting common render-target destructor.
011B2D91  mov     rbp, rsp
011B2D94  push    r14
011B2D96  push    rbx
011B2D97  lea     rax, unk_19A5B18
011B2D9E  mov     rbx, rdi
011B2DA1  add     rax, 10h
011B2DA5  mov     [rbx], rax
011B2DA8  cmp     byte ptr [rbx+0Ch], 0
011B2DAC  jz      short loc_11B2DCA
011B2DAE  mov     rdi, [rbx+10h]
011B2DB2  lea     r14, [rbx+10h]
011B2DB6  test    rdi, rdi
011B2DB9  jz      short loc_11B2DC1
011B2DBB  mov     rax, [rdi]
011B2DBE  call    qword ptr [rax+8]
011B2DC1  vxorps  xmm0, xmm0, xmm0
011B2DC5  vmovups xmmword ptr [r14], xmm0
011B2DCA  mov     rdi, rbx
011B2DCD  call    nullsub_43
011B2DD2  mov     rdi, rbx
011B2DD5  pop     rbx
011B2DD6  pop     r14
011B2DD8  pop     rbp
011B2DD9  jmp     sub_37BF50

; render_target_active_buffer_index at 0x11b2de0
011B2DE0  xor     eax, eax; Returns the common target's default active-buffer index of zero.
011B2DE2  retn

; render_target_active_state_handle at 0x11b2df0
011B2DF0  lea     rax, [rdi+18h]; Returns a handle to the active render-target state pointer.
011B2DF4  mov     edx, 1
011B2DF9  retn

; render_target_set_state at 0x11b2e00
011B2E00  mov     [rdi+10h], rsi; Sets both the owned and active target-state pointers.
011B2E04  mov     [rdi+18h], rsi
011B2E08  retn
