; render_texture_cube_construct @ 0x6A0DA0
006A0DA0  push    rbp; Constructs the exact 792-byte common cube texture object and six mip-chain faces.
006A0DA1  mov     rbp, rsp
006A0DA4  push    r14
006A0DA6  push    rbx
006A0DA7  mov     r14, rsi
006A0DAA  mov     rbx, rdi
006A0DAD  call    render_texture_construct
006A0DB2  lea     rax, unk_1935CD0
006A0DB9  mov     rdi, r14
006A0DBC  add     rax, 10h
006A0DC0  mov     [rbx], rax
006A0DC3  call    sub_69B990
006A0DC8  vmovups ymm0, ymmword ptr [r14+70h]
006A0DCE  lea     rdi, [rbx+138h]
006A0DD5  lea     rsi, [r14+90h]
006A0DDC  movzx   edx, al
006A0DDF  vmovups ymmword ptr [rbx+118h], ymm0
006A0DE7  vmovups ymm0, ymmword ptr [r14]
006A0DEC  vmovups ymm1, ymmword ptr [r14+20h]
006A0DF2  vmovups ymm2, ymmword ptr [r14+40h]
006A0DF8  vmovups ymm3, ymmword ptr [r14+60h]
006A0DFE  vmovups ymmword ptr [rbx+108h], ymm3
006A0E06  vmovups ymmword ptr [rbx+0E8h], ymm2
006A0E0E  vmovups ymmword ptr [rbx+0C8h], ymm1
006A0E16  vmovups ymmword ptr [rbx+0A8h], ymm0
006A0E1E  call    sub_68CC00
006A0E23  mov     dword ptr [rbx+0A0h], 2
006A0E2D  mov     eax, [r14+0A4h]
006A0E34  mov     [rbx+104h], eax
006A0E3A  mov     eax, [r14+0A0h]
006A0E41  mov     [rbx+110h], eax
006A0E47  mov     rax, [r14+98h]
006A0E4E  mov     [rbx+108h], rax
006A0E55  vmovups ymm0, ymmword ptr [rbx+118h]
006A0E5D  vmovups ymmword ptr [rbx+80h], ymm0
006A0E65  vmovups ymm0, ymmword ptr [rbx+0A8h]
006A0E6D  vmovups ymm1, ymmword ptr [rbx+0C8h]
006A0E75  vmovups ymm2, ymmword ptr [rbx+0E8h]
006A0E7D  vmovups ymm3, ymmword ptr [rbx+108h]
006A0E85  vmovups ymmword ptr [rbx+70h], ymm3
006A0E8A  vmovups ymmword ptr [rbx+50h], ymm2
006A0E8F  vmovups ymmword ptr [rbx+30h], ymm1
006A0E94  vmovups ymmword ptr [rbx+10h], ymm0
006A0E99  pop     rbx
006A0E9A  pop     r14
006A0E9C  pop     rbp
006A0E9D  retn

; render_texture_cube_destruct @ 0x6A0EA0
006A0EA0  push    rbp; Destroys all six cube-face mip chains and the RenderTexture base.
006A0EA1  mov     rbp, rsp
006A0EA4  push    r14
006A0EA6  push    rbx
006A0EA7  lea     rax, unk_1935CD0
006A0EAE  mov     rbx, rdi
006A0EB1  lea     rdi, [rbx+2C8h]
006A0EB8  lea     r14, [rbx+138h]
006A0EBF  add     rax, 10h
006A0EC3  mov     [rbx], rax
006A0EC6  call    sub_682BC0
006A0ECB  lea     rdi, [rbx+278h]
006A0ED2  call    sub_682BC0
006A0ED7  lea     rdi, [rbx+228h]
006A0EDE  call    sub_682BC0
006A0EE3  lea     rdi, [rbx+1D8h]
006A0EEA  call    sub_682BC0
006A0EEF  lea     rdi, [rbx+188h]
006A0EF6  call    sub_682BC0
006A0EFB  mov     rdi, r14
006A0EFE  call    sub_682BC0
006A0F03  mov     rdi, rbx
006A0F06  pop     rbx
006A0F07  pop     r14
006A0F09  pop     rbp
006A0F0A  jmp     render_texture_destruct

; render_texture_cube_delete @ 0x6A0F10
006A0F10  push    rbp; Deleting destructor for the common RenderTextureCube object.
006A0F11  mov     rbp, rsp
006A0F14  push    r14
006A0F16  push    rbx
006A0F17  lea     rax, unk_1935CD0
006A0F1E  mov     rbx, rdi
006A0F21  lea     rdi, [rbx+2C8h]
006A0F28  lea     r14, [rbx+138h]
006A0F2F  add     rax, 10h
006A0F33  mov     [rbx], rax
006A0F36  call    sub_682BC0
006A0F3B  lea     rdi, [rbx+278h]
006A0F42  call    sub_682BC0
006A0F47  lea     rdi, [rbx+228h]
006A0F4E  call    sub_682BC0
006A0F53  lea     rdi, [rbx+1D8h]
006A0F5A  call    sub_682BC0
006A0F5F  lea     rdi, [rbx+188h]
006A0F66  call    sub_682BC0
006A0F6B  mov     rdi, r14
006A0F6E  call    sub_682BC0
006A0F73  mov     rdi, rbx
006A0F76  call    render_texture_destruct
006A0F7B  mov     rdi, rbx
006A0F7E  pop     rbx
006A0F7F  pop     r14
006A0F81  pop     rbp
006A0F82  jmp     sub_37BF50
