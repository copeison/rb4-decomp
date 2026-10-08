; IDA disassembly evidence for the common render 3D-texture lifecycle.
; Functions: 0x6F5CB0-0x6F5E29

; render_texture_3d_construct at 0x6f5cb0
006F5CB0  push    rbp; Constructs the 392-byte common 3D texture from its 168-byte descriptor.
006F5CB1  mov     rbp, rsp
006F5CB4  push    r14
006F5CB6  push    rbx
006F5CB7  mov     r14, rsi
006F5CBA  mov     rbx, rdi
006F5CBD  call    render_texture_construct
006F5CC2  lea     rax, unk_1939D40
006F5CC9  mov     rdi, r14
006F5CCC  add     rax, 10h
006F5CD0  mov     [rbx], rax
006F5CD3  call    sub_69B990
006F5CD8  vmovups ymm0, ymmword ptr [r14+70h]
006F5CDE  lea     rdi, [rbx+138h]
006F5CE5  lea     rsi, [r14+90h]
006F5CEC  movzx   edx, al
006F5CEF  vmovups ymmword ptr [rbx+118h], ymm0
006F5CF7  vmovups ymm0, ymmword ptr [r14]
006F5CFC  vmovups ymm1, ymmword ptr [r14+20h]
006F5D02  vmovups ymm2, ymmword ptr [r14+40h]
006F5D08  vmovups ymm3, ymmword ptr [r14+60h]
006F5D0E  vmovups ymmword ptr [rbx+108h], ymm3
006F5D16  vmovups ymmword ptr [rbx+0E8h], ymm2
006F5D1E  vmovups ymmword ptr [rbx+0C8h], ymm1
006F5D26  vmovups ymmword ptr [rbx+0A8h], ymm0
006F5D2E  call    sub_682960
006F5D33  mov     eax, [r14+0A4h]
006F5D3A  mov     [rbx+104h], eax
006F5D40  mov     eax, [r14+0A0h]
006F5D47  mov     [rbx+110h], eax
006F5D4D  mov     rax, [r14+98h]
006F5D54  mov     [rbx+108h], rax
006F5D5B  vmovups ymm0, ymmword ptr [rbx+118h]
006F5D63  vmovups ymmword ptr [rbx+80h], ymm0
006F5D6B  vmovups ymm0, ymmword ptr [rbx+0A8h]
006F5D73  vmovups ymm1, ymmword ptr [rbx+0C8h]
006F5D7B  vmovups ymm2, ymmword ptr [rbx+0E8h]
006F5D83  vmovups ymm3, ymmword ptr [rbx+108h]
006F5D8B  vmovups ymmword ptr [rbx+70h], ymm3
006F5D90  vmovups ymmword ptr [rbx+50h], ymm2
006F5D95  vmovups ymmword ptr [rbx+30h], ymm1
006F5D9A  vmovups ymmword ptr [rbx+10h], ymm0
006F5D9F  pop     rbx
006F5DA0  pop     r14
006F5DA2  pop     rbp
006F5DA3  retn

; render_texture_3d_destruct at 0x6f5db0
006F5DB0  push    rbp; Destroys the common 3D texture mip-data state and common texture base.
006F5DB1  mov     rbp, rsp
006F5DB4  push    rbx
006F5DB5  push    rax
006F5DB6  lea     rax, unk_1939D40
006F5DBD  mov     rbx, rdi
006F5DC0  lea     rdi, [rbx+138h]
006F5DC7  add     rax, 10h
006F5DCB  mov     [rbx], rax
006F5DCE  call    sub_682BC0
006F5DD3  mov     rdi, rbx
006F5DD6  add     rsp, 8
006F5DDA  pop     rbx
006F5DDB  pop     rbp
006F5DDC  jmp     render_texture_destruct

; render_texture_3d_delete at 0x6f5df0
006F5DF0  push    rbp; Deleting common 3D texture destructor.
006F5DF1  mov     rbp, rsp
006F5DF4  push    rbx
006F5DF5  push    rax
006F5DF6  lea     rax, unk_1939D40
006F5DFD  mov     rbx, rdi
006F5E00  lea     rdi, [rbx+138h]
006F5E07  add     rax, 10h
006F5E0B  mov     [rbx], rax
006F5E0E  call    sub_682BC0
006F5E13  mov     rdi, rbx
006F5E16  call    render_texture_destruct
006F5E1B  mov     rdi, rbx
006F5E1E  add     rsp, 8
006F5E22  pop     rbx
006F5E23  pop     rbp
006F5E24  jmp     sub_37BF50
