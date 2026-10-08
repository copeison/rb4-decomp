; IDA disassembly evidence for the common render 2D texture-array lifecycle.
; Functions: 0x698160-0x6985C0

; render_texture_array_2d_construct at 0x698160
00698160  push    rbp; Constructs the 344-byte common 2D texture-array state and copies its 80-byte element mip chains.
00698161  mov     rbp, rsp
00698164  push    r15
00698166  push    r14
00698168  push    r13
0069816A  push    r12
0069816C  push    rbx
0069816D  sub     rsp, 48h
00698171  mov     rax, cs:qword_19A9B88
00698178  mov     r14, rsi
0069817B  mov     r13, rdi
0069817E  mov     rax, [rax]
00698181  mov     [rbp+var_30], rax
00698185  call    render_texture_construct
0069818A  lea     rax, unk_1935210
00698191  mov     rdi, r14
00698194  add     rax, 10h
00698198  mov     [r13+0], rax
0069819C  lea     rax, [r13+0A8h]
006981A3  mov     [rbp+var_68], rax
006981A7  call    sub_69B990
006981AC  mov     byte ptr [rbp+var_58], al
006981AF  lea     rdi, [r13+150h]
006981B6  lea     rsi, aEastlVector_251; "EASTL vector"
006981BD  lea     r12, [r13+138h]
006981C4  vmovups ymm0, ymmword ptr [r14+70h]
006981CA  vmovups ymmword ptr [r13+118h], ymm0
006981D3  vmovups ymm0, ymmword ptr [r14]
006981D8  vmovups ymm1, ymmword ptr [r14+20h]
006981DE  vmovups ymm2, ymmword ptr [r14+40h]
006981E4  vmovups ymm3, ymmword ptr [r14+60h]
006981EA  vmovups ymmword ptr [r13+108h], ymm3
006981F3  vmovups ymmword ptr [r13+0E8h], ymm2
006981FC  vmovups ymmword ptr [r13+0C8h], ymm1
00698205  vmovups ymmword ptr [r13+0A8h], ymm0
0069820E  vxorps  xmm0, xmm0, xmm0
00698212  vmovups xmmword ptr [r13+138h], xmm0
0069821B  mov     qword ptr [r13+148h], 0
00698226  call    nullsub_18
0069822B  mov     rsi, [r14+98h]
00698232  mov     rax, 0CCCCCCCCCCCCCCCDh
0069823C  mov     rdi, r12
0069823F  sub     rsi, [r14+90h]
00698246  sar     rsi, 4
0069824A  imul    rsi, rax
0069824E  call    sub_697470
00698253  mov     rbx, [r14+90h]
0069825A  mov     [rbp+var_70], r14
0069825E  mov     r14, [r14+98h]
00698265  cmp     rbx, r14
00698268  jz      short loc_6982B7
0069826A  lea     r15, [rbp+var_58]
0069826E  xchg    ax, ax
00698270  mov     rdi, [r13+140h]
00698277  cmp     rdi, [r13+148h]
0069827E  jnb     short loc_6982A0
00698280  movzx   edx, byte ptr [rbp+var_58]
00698284  mov     rsi, rbx
00698287  call    sub_682960
0069828C  add     qword ptr [r13+140h], 50h ; 'P'
00698294  jmp     short loc_6982AE
00698296  align 20h
006982A0  mov     rdi, r12
006982A3  mov     rsi, rbx
006982A6  mov     rdx, r15
006982A9  call    sub_697890
006982AE  add     rbx, 50h ; 'P'
006982B2  cmp     r14, rbx
006982B5  jnz     short loc_698270
006982B7  lea     rax, unk_19C8208
006982BE  lea     rcx, unk_18EFB28
006982C5  lea     r15, unk_18DCC08
006982CC  vxorps  xmm0, xmm0, xmm0
006982D0  lea     rdi, [rbp+var_60]
006982D4  vmovups [rbp+var_50], xmm0
006982D9  mov     [rbp+var_40], 8
006982E0  mov     [rbp+var_3C], 0
006982E7  mov     rsi, [rcx]
006982EA  mov     rax, [rax]
006982ED  add     r15, 10h
006982F1  mov     [rbp+var_38], rax
006982F5  mov     [rbp+var_58], r15
006982F9  mov     [rbp+var_60], rsi
006982FD  call    sub_1AF950
00698302  mov     rbx, [rbp+var_68]
00698306  mov     rsi, [rbp+var_60]
0069830A  lea     r14, [rbp+var_58]
0069830E  xor     ecx, ecx
00698310  mov     rdx, r14
00698313  mov     rdi, rbx
00698316  call    sub_698910
0069831B  xor     esi, esi
0069831D  mov     rdi, r14
00698320  mov     [rbp+var_58], r15
00698324  call    sub_A660
00698329  cmp     [rbp+var_3C], 0
0069832D  jnz     short loc_698338
0069832F  mov     rdi, qword ptr [rbp+var_50]
00698333  call    sub_37B800
00698338  mov     dword ptr [r13+0A0h], 0
00698343  mov     rdx, [rbp+var_70]
00698347  mov     rax, [rdx+90h]
0069834E  mov     ecx, [rax+14h]
00698351  mov     [r13+104h], ecx
00698358  mov     ecx, [rax+10h]
0069835B  mov     [r13+110h], ecx
00698362  mov     rcx, 0CCCCCCCCCCCCCCCDh
0069836C  mov     rax, [rax+8]
00698370  mov     [r13+108h], rax
00698377  mov     rax, [rdx+98h]
0069837E  sub     rax, [rdx+90h]
00698385  sar     rax, 4
00698389  imul    rax, rcx
0069838D  mov     [r13+118h], rax
00698394  mov     rax, cs:qword_19A9B88
0069839B  vmovups ymm0, ymmword ptr [rbx+70h]
006983A0  vmovups ymmword ptr [r13+80h], ymm0
006983A9  vmovups ymm0, ymmword ptr [rbx]
006983AD  vmovups ymm1, ymmword ptr [rbx+20h]
006983B2  vmovups ymm2, ymmword ptr [rbx+40h]
006983B7  vmovups ymm3, ymmword ptr [rbx+60h]
006983BC  vmovups ymmword ptr [r13+70h], ymm3
006983C2  vmovups ymmword ptr [r13+50h], ymm2
006983C8  vmovups ymmword ptr [r13+30h], ymm1
006983CE  vmovups ymmword ptr [r13+10h], ymm0
006983D4  mov     rax, [rax]
006983D7  cmp     rax, [rbp+var_30]
006983DB  jnz     short loc_6983EC
006983DD  add     rsp, 48h
006983E1  pop     rbx
006983E2  pop     r12
006983E4  pop     r13
006983E6  pop     r14
006983E8  pop     r15
006983EA  pop     rbp
006983EB  retn
006983EC  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a

; render_texture_array_2d_destruct at 0x6984c0
006984C0  push    rbp; Destroys each common 2D texture-array mip chain, releases vector storage, and tears down the common texture base.
006984C1  mov     rbp, rsp
006984C4  push    r15
006984C6  push    r14
006984C8  push    rbx
006984C9  push    rax
006984CA  lea     rax, unk_1935210
006984D1  mov     r14, rdi
006984D4  add     rax, 10h
006984D8  mov     [r14], rax
006984DB  mov     rbx, [r14+138h]
006984E2  mov     r15, [r14+140h]
006984E9  cmp     rbx, r15
006984EC  jz      short loc_698508
006984EE  xchg    ax, ax
006984F0  mov     rax, [rbx]
006984F3  mov     rdi, rbx
006984F6  call    qword ptr [rax]
006984F8  add     rbx, 50h ; 'P'
006984FC  cmp     r15, rbx
006984FF  jnz     short loc_6984F0
00698501  mov     rbx, [r14+138h]
00698508  test    rbx, rbx
0069850B  jz      short loc_698526
0069850D  mov     rdx, [r14+148h]
00698514  lea     rdi, [r14+150h]
0069851B  mov     rsi, rbx
0069851E  sub     rdx, rbx
00698521  call    sub_252D30
00698526  mov     rdi, r14
00698529  add     rsp, 8
0069852D  pop     rbx
0069852E  pop     r14
00698530  pop     r15
00698532  pop     rbp
00698533  jmp     render_texture_destruct

; render_texture_array_2d_delete at 0x698540
00698540  push    rbp; Deleting common 2D texture-array destructor.
00698541  mov     rbp, rsp
00698544  push    r15
00698546  push    r14
00698548  push    rbx
00698549  push    rax
0069854A  lea     rax, unk_1935210
00698551  mov     r14, rdi
00698554  add     rax, 10h
00698558  mov     [r14], rax
0069855B  mov     rbx, [r14+138h]
00698562  mov     r15, [r14+140h]
00698569  cmp     rbx, r15
0069856C  jz      short loc_698588
0069856E  xchg    ax, ax
00698570  mov     rax, [rbx]
00698573  mov     rdi, rbx
00698576  call    qword ptr [rax]
00698578  add     rbx, 50h ; 'P'
0069857C  cmp     r15, rbx
0069857F  jnz     short loc_698570
00698581  mov     rbx, [r14+138h]
00698588  test    rbx, rbx
0069858B  jz      short loc_6985A6
0069858D  mov     rdx, [r14+148h]
00698594  lea     rdi, [r14+150h]
0069859B  mov     rsi, rbx
0069859E  sub     rdx, rbx
006985A1  call    sub_252D30
006985A6  mov     rdi, r14
006985A9  call    render_texture_destruct
006985AE  mov     rdi, r14
006985B1  add     rsp, 8
006985B5  pop     rbx
006985B6  pop     r14
006985B8  pop     r15
006985BA  pop     rbp
006985BB  jmp     sub_37BF50
