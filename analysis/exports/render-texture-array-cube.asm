; IDA disassembly evidence for the common render cube texture-array lifecycle.
; Functions: 0x69AAC0-0x69AD49

; render_texture_array_cube_construct at 0x69aac0
0069AAC0  push    rbp; Constructs the 344-byte common cube texture-array state and normalizes its 480-byte cube records.
0069AAC1  mov     rbp, rsp
0069AAC4  push    r15
0069AAC6  push    r14
0069AAC8  push    rbx
0069AAC9  push    rax
0069AACA  mov     r14, rsi
0069AACD  mov     rbx, rdi
0069AAD0  call    render_texture_construct
0069AAD5  lea     rax, unk_19355C0
0069AADC  mov     rdi, r14
0069AADF  lea     r15, [rbx+0A8h]
0069AAE6  add     rax, 10h
0069AAEA  mov     [rbx], rax
0069AAED  call    sub_69B990
0069AAF2  movzx   edx, al
0069AAF5  mov     rdi, r15
0069AAF8  mov     rsi, r14
0069AAFB  call    sub_69AF50
0069AB00  mov     rdi, r15
0069AB03  call    sub_69ABB0
0069AB08  mov     dword ptr [rbx+0A0h], 2
0069AB12  mov     rax, [r14+90h]
0069AB19  mov     ecx, [rax+14h]
0069AB1C  mov     [rbx+104h], ecx
0069AB22  mov     ecx, [rax+10h]
0069AB25  mov     [rbx+110h], ecx
0069AB2B  mov     rcx, 0EEEEEEEEEEEEEEEFh
0069AB35  mov     rax, [rax+8]
0069AB39  mov     [rbx+108h], rax
0069AB40  mov     rax, [r14+98h]
0069AB47  sub     rax, [r14+90h]
0069AB4E  sar     rax, 5
0069AB52  imul    rcx, rax
0069AB56  mov     [rbx+118h], rcx
0069AB5D  vmovups ymm0, ymmword ptr [rbx+118h]
0069AB65  vmovups ymmword ptr [rbx+80h], ymm0
0069AB6D  vmovups ymm0, ymmword ptr [rbx+0A8h]
0069AB75  vmovups ymm1, ymmword ptr [rbx+0C8h]
0069AB7D  vmovups ymm2, ymmword ptr [rbx+0E8h]
0069AB85  vmovups ymm3, ymmword ptr [rbx+108h]
0069AB8D  vmovups ymmword ptr [rbx+70h], ymm3
0069AB92  vmovups ymmword ptr [rbx+50h], ymm2
0069AB97  vmovups ymmword ptr [rbx+30h], ymm1
0069AB9C  vmovups ymmword ptr [rbx+10h], ymm0
0069ABA1  add     rsp, 8
0069ABA5  pop     rbx
0069ABA6  pop     r14
0069ABA8  pop     r15
0069ABAA  pop     rbp
0069ABAB  retn

; render_texture_array_cube_destruct at 0x69acd0
0069ACD0  push    rbp; Destroys the common cube-array record vector and common texture base.
0069ACD1  mov     rbp, rsp
0069ACD4  push    rbx
0069ACD5  push    rax
0069ACD6  lea     rax, unk_19355C0
0069ACDD  mov     rbx, rdi
0069ACE0  lea     rdi, [rbx+138h]
0069ACE7  add     rax, 10h
0069ACEB  mov     [rbx], rax
0069ACEE  call    sub_48D3D0
0069ACF3  mov     rdi, rbx
0069ACF6  add     rsp, 8
0069ACFA  pop     rbx
0069ACFB  pop     rbp
0069ACFC  jmp     render_texture_destruct

; render_texture_array_cube_delete at 0x69ad10
0069AD10  push    rbp; Deleting common cube texture-array destructor.
0069AD11  mov     rbp, rsp
0069AD14  push    rbx
0069AD15  push    rax
0069AD16  lea     rax, unk_19355C0
0069AD1D  mov     rbx, rdi
0069AD20  lea     rdi, [rbx+138h]
0069AD27  add     rax, 10h
0069AD2B  mov     [rbx], rax
0069AD2E  call    sub_48D3D0
0069AD33  mov     rdi, rbx
0069AD36  call    render_texture_destruct
0069AD3B  mov     rdi, rbx
0069AD3E  add     rsp, 8
0069AD42  pop     rbx
0069AD43  pop     rbp
0069AD44  jmp     sub_37BF50
