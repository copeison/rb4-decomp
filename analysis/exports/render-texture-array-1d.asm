; IDA disassembly evidence for the common render 1D texture-array lifecycle.
; Functions: 0x696C00-0x697120

; render_texture_array_1d_construct at 0x696c00
00696C00  push    rbp; Constructs the 344-byte common 1D texture-array state and copies its 80-byte element mip chains.
00696C01  mov     rbp, rsp
00696C04  push    r15
00696C06  push    r14
00696C08  push    r13
00696C0A  push    r12
00696C0C  push    rbx
00696C0D  sub     rsp, 28h
00696C11  mov     rax, cs:qword_19A9B88
00696C18  mov     r14, rsi
00696C1B  mov     r12, rdi
00696C1E  mov     rax, [rax]
00696C21  mov     [rbp+var_30], rax
00696C25  call    render_texture_construct
00696C2A  lea     rax, unk_19350E0
00696C31  mov     rdi, r14
00696C34  add     rax, 10h
00696C38  mov     [r12], rax
00696C3C  call    sub_69B990
00696C41  mov     [rbp+var_31], al
00696C44  lea     rdi, [r12+150h]
00696C4C  lea     rsi, aEastlVector_250; "EASTL vector"
00696C53  lea     r15, [r12+138h]
00696C5B  vmovups ymm0, ymmword ptr [r14+70h]
00696C61  vmovups ymmword ptr [r12+118h], ymm0
00696C6B  vmovups ymm0, ymmword ptr [r14]
00696C70  vmovups ymm1, ymmword ptr [r14+20h]
00696C76  vmovups ymm2, ymmword ptr [r14+40h]
00696C7C  vmovups ymm3, ymmword ptr [r14+60h]
00696C82  vmovups ymmword ptr [r12+108h], ymm3
00696C8C  vmovups ymmword ptr [r12+0E8h], ymm2
00696C96  vmovups ymmword ptr [r12+0C8h], ymm1
00696CA0  vmovups ymmword ptr [r12+0A8h], ymm0
00696CAA  vxorps  xmm0, xmm0, xmm0
00696CAE  vmovups xmmword ptr [r12+138h], xmm0
00696CB8  mov     qword ptr [r12+148h], 0
00696CC4  call    nullsub_18
00696CC9  mov     rsi, [r14+98h]
00696CD0  mov     rax, 0CCCCCCCCCCCCCCCDh
00696CDA  mov     rdi, r15
00696CDD  sub     rsi, [r14+90h]
00696CE4  sar     rsi, 4
00696CE8  imul    rsi, rax
00696CEC  call    sub_697470
00696CF1  mov     rbx, [r14+90h]
00696CF8  mov     [rbp+var_48], r14
00696CFC  mov     r14, [r14+98h]
00696D03  cmp     rbx, r14
00696D06  jz      short loc_696D57
00696D08  lea     r13, [rbp+var_31]
00696D0C  nop     dword ptr [rax+00h]
00696D10  mov     rdi, [r12+140h]
00696D18  cmp     rdi, [r12+148h]
00696D20  jnb     short loc_696D40
00696D22  movzx   edx, [rbp+var_31]
00696D26  mov     rsi, rbx
00696D29  call    sub_682960
00696D2E  add     qword ptr [r12+140h], 50h ; 'P'
00696D37  jmp     short loc_696D4E
00696D39  align 20h
00696D40  mov     rdi, r15
00696D43  mov     rsi, rbx
00696D46  mov     rdx, r13
00696D49  call    sub_697890
00696D4E  add     rbx, 50h ; 'P'
00696D52  cmp     r14, rbx
00696D55  jnz     short loc_696D10
00696D57  mov     rsi, [r12+138h]
00696D5F  mov     rax, [r12+140h]
00696D67  mov     r14, 0CCCCCCCCCCCCCCCDh
00696D71  lea     rbx, [r12+0A8h]
00696D79  mov     r8, r14
00696D7C  cmp     rsi, rax
00696D7F  jz      loc_696E55
00696D85  sub     rax, rsi
00696D88  sar     rax, 4
00696D8C  imul    rax, r8
00696D90  cmp     rax, 800h
00696D96  ja      loc_696E55
00696D9C  lea     rcx, [rsi+10h]
00696DA0  xor     edx, edx
00696DA2  nop     word ptr [rax+rax+00000000h]
00696DB0  cmp     dword ptr [rcx-4], 1
00696DB4  jnz     loc_696E55
00696DBA  cmp     dword ptr [rcx], 1
00696DBD  jnz     loc_696E55
00696DC3  inc     rdx
00696DC6  add     rcx, 50h ; 'P'
00696DCA  cmp     rdx, rax
00696DCD  jb      short loc_696DB0
00696DCF  cmp     rax, 2
00696DD3  jb      loc_696E55
00696DD9  mov     r13d, 1
00696DDF  mov     r15d, 50h ; 'P'
00696DE5  mov     rdi, rsi
00696DE8  nop     dword ptr [rax+rax+00000000h]
00696DF0  mov     eax, [rdi+r15+8]
00696DF5  cmp     eax, [rsi+8]
00696DF8  jnz     short loc_696E55
00696DFA  mov     eax, [rdi+r15+14h]
00696DFF  cmp     eax, [rsi+14h]
00696E02  jnz     short loc_696E55
00696E04  add     rdi, r15
00696E07  mov     r14, rsi
00696E0A  call    sub_6832D0
00696E0F  mov     rdi, r14
00696E12  mov     [rbp+var_40], rax
00696E16  call    sub_6832D0
00696E1B  cmp     [rbp+var_40], rax
00696E1F  mov     r8, 0CCCCCCCCCCCCCCCDh
00696E29  mov     rsi, r14
00696E2C  jnz     short loc_696E55
00696E2E  mov     rdi, [r12+138h]
00696E36  mov     rax, [r12+140h]
00696E3E  inc     r13
00696E41  add     r15, 50h ; 'P'
00696E45  sub     rax, rdi
00696E48  sar     rax, 4
00696E4C  imul    rax, r8
00696E50  cmp     r13, rax
00696E53  jb      short loc_696DF0
00696E55  mov     rdx, [rbp+var_48]
00696E59  mov     rax, [rdx+90h]
00696E60  mov     ecx, [rax+14h]
00696E63  mov     [r12+104h], ecx
00696E6B  mov     ecx, [rax+10h]
00696E6E  mov     [r12+110h], ecx
00696E76  mov     rax, [rax+8]
00696E7A  mov     [r12+108h], rax
00696E82  mov     rax, [rdx+98h]
00696E89  sub     rax, [rdx+90h]
00696E90  sar     rax, 4
00696E94  imul    rax, r8
00696E98  mov     [r12+118h], rax
00696EA0  mov     rax, cs:qword_19A9B88
00696EA7  vmovups ymm0, ymmword ptr [rbx+70h]
00696EAC  vmovups ymmword ptr [r12+80h], ymm0
00696EB6  vmovups ymm0, ymmword ptr [rbx]
00696EBA  vmovups ymm1, ymmword ptr [rbx+20h]
00696EBF  vmovups ymm2, ymmword ptr [rbx+40h]
00696EC4  vmovups ymm3, ymmword ptr [rbx+60h]
00696EC9  vmovups ymmword ptr [r12+70h], ymm3
00696ED0  vmovups ymmword ptr [r12+50h], ymm2
00696ED7  vmovups ymmword ptr [r12+30h], ymm1
00696EDE  vmovups ymmword ptr [r12+10h], ymm0
00696EE5  mov     rax, [rax]
00696EE8  cmp     rax, [rbp+var_30]
00696EEC  jnz     short loc_696EFD
00696EEE  add     rsp, 28h
00696EF2  pop     rbx
00696EF3  pop     r12
00696EF5  pop     r13
00696EF7  pop     r14
00696EF9  pop     r15
00696EFB  pop     rbp
00696EFC  retn
00696EFD  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a

; render_texture_array_1d_destruct at 0x697020
00697020  push    rbp; Destroys each common 1D texture-array mip chain, releases vector storage, and tears down the common texture base.
00697021  mov     rbp, rsp
00697024  push    r15
00697026  push    r14
00697028  push    rbx
00697029  push    rax
0069702A  lea     rax, unk_19350E0
00697031  mov     r14, rdi
00697034  add     rax, 10h
00697038  mov     [r14], rax
0069703B  mov     rbx, [r14+138h]
00697042  mov     r15, [r14+140h]
00697049  cmp     rbx, r15
0069704C  jz      short loc_697068
0069704E  xchg    ax, ax
00697050  mov     rax, [rbx]
00697053  mov     rdi, rbx
00697056  call    qword ptr [rax]
00697058  add     rbx, 50h ; 'P'
0069705C  cmp     r15, rbx
0069705F  jnz     short loc_697050
00697061  mov     rbx, [r14+138h]
00697068  test    rbx, rbx
0069706B  jz      short loc_697086
0069706D  mov     rdx, [r14+148h]
00697074  lea     rdi, [r14+150h]
0069707B  mov     rsi, rbx
0069707E  sub     rdx, rbx
00697081  call    sub_252D30
00697086  mov     rdi, r14
00697089  add     rsp, 8
0069708D  pop     rbx
0069708E  pop     r14
00697090  pop     r15
00697092  pop     rbp
00697093  jmp     render_texture_destruct

; render_texture_array_1d_delete at 0x6970a0
006970A0  push    rbp; Deleting common 1D texture-array destructor.
006970A1  mov     rbp, rsp
006970A4  push    r15
006970A6  push    r14
006970A8  push    rbx
006970A9  push    rax
006970AA  lea     rax, unk_19350E0
006970B1  mov     r14, rdi
006970B4  add     rax, 10h
006970B8  mov     [r14], rax
006970BB  mov     rbx, [r14+138h]
006970C2  mov     r15, [r14+140h]
006970C9  cmp     rbx, r15
006970CC  jz      short loc_6970E8
006970CE  xchg    ax, ax
006970D0  mov     rax, [rbx]
006970D3  mov     rdi, rbx
006970D6  call    qword ptr [rax]
006970D8  add     rbx, 50h ; 'P'
006970DC  cmp     r15, rbx
006970DF  jnz     short loc_6970D0
006970E1  mov     rbx, [r14+138h]
006970E8  test    rbx, rbx
006970EB  jz      short loc_697106
006970ED  mov     rdx, [r14+148h]
006970F4  lea     rdi, [r14+150h]
006970FB  mov     rsi, rbx
006970FE  sub     rdx, rbx
00697101  call    sub_252D30
00697106  mov     rdi, r14
00697109  call    render_texture_destruct
0069710E  mov     rdi, r14
00697111  add     rsp, 8
00697115  pop     rbx
00697116  pop     r14
00697118  pop     r15
0069711A  pop     rbp
0069711B  jmp     sub_37BF50
