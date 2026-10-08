; IDA disassembly evidence for the common render 1D-texture lifecycle.
; Functions: 0x6F5870-0x6F59E9

; render_texture_1d_construct at 0x6f5870
006F5870  push    rbp; Constructs the 392-byte common 1D texture from its 168-byte descriptor.
006F5871  mov     rbp, rsp
006F5874  push    r14
006F5876  push    rbx
006F5877  mov     r14, rsi
006F587A  mov     rbx, rdi
006F587D  call    render_texture_construct
006F5882  lea     rax, unk_1939C88
006F5889  mov     rdi, r14
006F588C  add     rax, 10h
006F5890  mov     [rbx], rax
006F5893  call    sub_69B990
006F5898  vmovups ymm0, ymmword ptr [r14+70h]
006F589E  lea     rdi, [rbx+138h]
006F58A5  lea     rsi, [r14+90h]
006F58AC  movzx   edx, al
006F58AF  vmovups ymmword ptr [rbx+118h], ymm0
006F58B7  vmovups ymm0, ymmword ptr [r14]
006F58BC  vmovups ymm1, ymmword ptr [r14+20h]
006F58C2  vmovups ymm2, ymmword ptr [r14+40h]
006F58C8  vmovups ymm3, ymmword ptr [r14+60h]
006F58CE  vmovups ymmword ptr [rbx+108h], ymm3
006F58D6  vmovups ymmword ptr [rbx+0E8h], ymm2
006F58DE  vmovups ymmword ptr [rbx+0C8h], ymm1
006F58E6  vmovups ymmword ptr [rbx+0A8h], ymm0
006F58EE  call    sub_682960
006F58F3  mov     eax, [r14+0A4h]
006F58FA  mov     [rbx+104h], eax
006F5900  mov     eax, [r14+0A0h]
006F5907  mov     [rbx+110h], eax
006F590D  mov     rax, [r14+98h]
006F5914  mov     [rbx+108h], rax
006F591B  vmovups ymm0, ymmword ptr [rbx+118h]
006F5923  vmovups ymmword ptr [rbx+80h], ymm0
006F592B  vmovups ymm0, ymmword ptr [rbx+0A8h]
006F5933  vmovups ymm1, ymmword ptr [rbx+0C8h]
006F593B  vmovups ymm2, ymmword ptr [rbx+0E8h]
006F5943  vmovups ymm3, ymmword ptr [rbx+108h]
006F594B  vmovups ymmword ptr [rbx+70h], ymm3
006F5950  vmovups ymmword ptr [rbx+50h], ymm2
006F5955  vmovups ymmword ptr [rbx+30h], ymm1
006F595A  vmovups ymmword ptr [rbx+10h], ymm0
006F595F  pop     rbx
006F5960  pop     r14
006F5962  pop     rbp
006F5963  retn

; render_texture_1d_destruct at 0x6f5970
006F5970  push    rbp; Destroys the common 1D texture mip-data state and common texture base.
006F5971  mov     rbp, rsp
006F5974  push    rbx
006F5975  push    rax
006F5976  lea     rax, unk_1939C88
006F597D  mov     rbx, rdi
006F5980  lea     rdi, [rbx+138h]
006F5987  add     rax, 10h
006F598B  mov     [rbx], rax
006F598E  call    sub_682BC0
006F5993  mov     rdi, rbx
006F5996  add     rsp, 8
006F599A  pop     rbx
006F599B  pop     rbp
006F599C  jmp     render_texture_destruct

; render_texture_1d_delete at 0x6f59b0
006F59B0  push    rbp; Deleting common 1D texture destructor.
006F59B1  mov     rbp, rsp
006F59B4  push    rbx
006F59B5  push    rax
006F59B6  lea     rax, unk_1939C88
006F59BD  mov     rbx, rdi
006F59C0  lea     rdi, [rbx+138h]
006F59C7  add     rax, 10h
006F59CB  mov     [rbx], rax
006F59CE  call    sub_682BC0
006F59D3  mov     rdi, rbx
006F59D6  call    render_texture_destruct
006F59DB  mov     rdi, rbx
006F59DE  add     rsp, 8
006F59E2  pop     rbx
006F59E3  pop     rbp
006F59E4  jmp     sub_37BF50
