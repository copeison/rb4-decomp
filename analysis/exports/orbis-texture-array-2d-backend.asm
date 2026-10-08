; IDA disassembly evidence for the Orbis 2D texture-array backend.
; Range: 0x8E5D80-0x8E648F

008E5D80  push    rbp
008E5D81  mov     rbp, rsp
008E5D84  push    r14
008E5D86  push    rbx
008E5D87  lea     rax, unk_195FB88
008E5D8E  lea     r14, g_orbis_render_system
008E5D95  mov     rbx, rdi
008E5D98  add     rax, 10h
008E5D9C  mov     [rbx], rax
008E5D9F  mov     rdi, [r14]
008E5DA2  mov     rsi, [rbx+160h]
008E5DA9  call    orbis_defer_allocation_release
008E5DAE  mov     rdi, [r14]
008E5DB1  mov     rsi, [rbx+168h]
008E5DB8  call    orbis_defer_allocation_release
008E5DBD  mov     rdi, [r14]
008E5DC0  mov     rsi, [rbx+170h]
008E5DC7  call    orbis_defer_allocation_release
008E5DCC  mov     rax, [rbx+178h]
008E5DD3  test    rax, rax
008E5DD6  jz      short loc_8E5E03
008E5DD8  mov     esi, [rax+1Ch]
008E5DDB  mov     rdi, [r14]
008E5DDE  shl     rsi, 8
008E5DE2  call    orbis_defer_allocation_release
008E5DE7  mov     rdi, [rbx+178h]
008E5DEE  test    rdi, rdi
008E5DF1  jz      short loc_8E5DF8
008E5DF3  call    sub_37BF50
008E5DF8  mov     qword ptr [rbx+178h], 0
008E5E03  mov     rdi, [rbx+158h]
008E5E0A  test    rdi, rdi
008E5E0D  jz      short loc_8E5E14
008E5E0F  call    sub_37BF50
008E5E14  mov     qword ptr [rbx+158h], 0
008E5E1F  mov     rdi, [rbx+180h]
008E5E26  test    rdi, rdi
008E5E29  jz      short loc_8E5E30
008E5E2B  call    sub_37BF50
008E5E30  mov     qword ptr [rbx+180h], 0
008E5E3B  mov     rdi, rbx
008E5E3E  pop     rbx
008E5E3F  pop     r14
008E5E41  pop     rbp
008E5E42  jmp     loc_6984C0
008E5E47  align 10h
008E5E50  push    rbp
008E5E51  mov     rbp, rsp
008E5E54  push    rbx
008E5E55  push    rax
008E5E56  mov     rbx, rdi
008E5E59  call    sub_8E5D80
008E5E5E  mov     rdi, rbx
008E5E61  add     rsp, 8
008E5E65  pop     rbx
008E5E66  pop     rbp
008E5E67  jmp     sub_37BF50
008E5E6C  align 10h
008E5E70  mov     rdx, [rdi+138h]
008E5E77  cmp     dword ptr [rdi+40h], 2
008E5E7B  lea     rsi, [rdi+0A8h]
008E5E82  jnz     short loc_8E5E89
008E5E84  jmp     loc_8E5E90
008E5E89  jmp     loc_8E6140
008E5E8E  align 10h
008E5E90  push    rbp
008E5E91  mov     rbp, rsp
008E5E94  push    r15
008E5E96  push    r14
008E5E98  push    r13
008E5E9A  push    r12
008E5E9C  push    rbx
008E5E9D  sub     rsp, 38h
008E5EA1  mov     rax, cs:qword_19A9B88
008E5EA8  mov     r12, rsi
008E5EAB  mov     r14, rdi
008E5EAE  mov     rax, [rax]
008E5EB1  mov     [rbp-30h], rax
008E5EB5  mov     ebx, [rdx+14h]
008E5EB8  call    sub_10DEF20
008E5EBD  mov     edi, ebx
008E5EBF  mov     r15d, eax
008E5EC2  call    sub_8E17C0
008E5EC7  mov     edi, ebx
008E5EC9  mov     r13d, eax
008E5ECC  call    sub_8E17F0
008E5ED1  mov     edi, r13d
008E5ED4  mov     ebx, eax
008E5ED6  call    sub_10C2C20
008E5EDB  cmp     ebx, 1
008E5EDE  mov     edx, 3
008E5EE3  lea     rsi, [rbp-34h]
008E5EE7  mov     r8d, 1
008E5EED  mov     edi, r15d
008E5EF0  mov     ecx, eax
008E5EF2  adc     edx, 0
008E5EF5  call    sub_10F4A00
008E5EFA  lea     rdi, [rbp-60h]
008E5EFE  call    sub_10C3220
008E5F03  mov     eax, [r14+70h]
008E5F07  mov     ecx, [rbp-34h]
008E5F0A  mov     edi, 34h ; '4'
008E5F0F  mov     [rbp-60h], eax
008E5F12  mov     eax, [r14+74h]
008E5F16  mov     [rbp-5Ch], eax
008E5F19  mov     dword ptr [rbp-58h], 0
008E5F20  mov     rax, [r12+98h]
008E5F28  sub     rax, [r12+90h]
008E5F30  shr     rax, 4
008E5F34  imul    eax, 0CCCCCCCDh
008E5F3A  mov     [rbp-54h], eax
008E5F3D  mov     [rbp-4Ch], ebx
008E5F40  mov     [rbp-50h], r13d
008E5F44  mov     [rbp-48h], ecx
008E5F47  mov     dword ptr [rbp-40h], 0
008E5F4E  or      byte ptr [rbp-3Ch], 1
008E5F52  mov     [rbp-44h], r15d
008E5F56  call    sub_37BF40
008E5F5B  vxorps  ymm0, ymm0, ymm0
008E5F5F  mov     rdi, rax
008E5F62  lea     rsi, [rbp-60h]
008E5F66  vmovups ymmword ptr [rax+14h], ymm0
008E5F6B  vmovups ymmword ptr [rax], ymm0
008E5F6F  mov     [r14+180h], rax
008E5F76  call    sub_10C3270
008E5F7B  mov     rdi, [r14+180h]
008E5F82  call    sub_10C3A30
008E5F87  mov     cl, cs:byte_1ADF270
008E5F8D  mov     r12, rax
008E5F90  mov     r15, r12
008E5F93  shr     r15, 20h
008E5F97  test    cl, cl
008E5F99  jnz     short loc_8E5FCA
008E5F9B  lea     rdi, byte_1ADF270
008E5FA2  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E5FA7  test    eax, eax
008E5FA9  jz      short loc_8E5FCA
008E5FAB  lea     rdi, aGpu_11; "gpu"
008E5FB2  call    sub_37BA70
008E5FB7  lea     rdi, byte_1ADF270
008E5FBE  mov     cs:qword_1ADF268, rax
008E5FC5  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E5FCA  mov     rdi, cs:qword_1ADF268
008E5FD1  call    sub_37A920
008E5FD6  mov     rsi, [r14+98h]
008E5FDD  mov     edi, r12d
008E5FE0  mov     edx, r15d
008E5FE3  call    sub_37AE70
008E5FE8  mov     r12d, ebx
008E5FEB  test    ebx, ebx
008E5FED  mov     [r14+160h], rax
008E5FF4  jz      short loc_8E601E
008E5FF6  mov     rdi, [r14+180h]
008E5FFD  call    sub_10C3AE0
008E6002  mov     rsi, [r14+98h]
008E6009  mov     rdx, rax
008E600C  mov     edi, eax
008E600E  shr     rdx, 20h
008E6012  call    sub_37AE70
008E6017  mov     [r14+168h], rax
008E601E  mov     rdi, [r14+180h]
008E6025  call    sub_10C3B90
008E602A  mov     rsi, [r14+98h]
008E6031  mov     rdx, rax
008E6034  mov     edi, eax
008E6036  shr     rdx, 20h
008E603A  call    sub_37AE70
008E603F  mov     [r14+170h], rax
008E6046  call    sub_37A9B0
008E604B  mov     rbx, [r14+160h]
008E6052  mov     r15, [r14+180h]
008E6059  test    r12d, r12d
008E605C  jz      short loc_8E6099
008E605E  mov     r12, [r14+168h]
008E6065  shr     rbx, 8
008E6069  mov     rdi, r15
008E606C  mov     esi, ebx
008E606E  call    sub_10C48C0
008E6073  mov     rdi, r15
008E6076  mov     esi, ebx
008E6078  call    sub_10C4B80
008E607D  shr     r12, 8
008E6081  mov     rdi, r15
008E6084  mov     esi, r12d
008E6087  call    sub_10C4E40
008E608C  mov     rdi, r15
008E608F  mov     esi, r12d
008E6092  call    sub_10C5130
008E6097  jmp     short loc_8E60BE
008E6099  shr     rbx, 8
008E609D  mov     rdi, r15
008E60A0  mov     esi, ebx
008E60A2  call    sub_10C48C0
008E60A7  mov     rsi, [r14+160h]
008E60AE  mov     rdi, [r14+180h]
008E60B5  shr     rsi, 8
008E60B9  call    sub_10C4B80
008E60BE  mov     rax, [r14+170h]
008E60C5  mov     rbx, [r14+180h]
008E60CC  mov     edi, 20h ; ' '
008E60D1  shr     rax, 8
008E60D5  mov     [rbx+24h], eax
008E60D8  mov     eax, 5FFFFFFFh
008E60DD  and     eax, [rbx]
008E60DF  or      eax, 20000000h
008E60E4  mov     [rbx], eax
008E60E6  call    sub_37BF40
008E60EB  vxorps  ymm0, ymm0, ymm0
008E60EF  xor     edx, edx
008E60F1  mov     rdi, rax
008E60F4  mov     rsi, rbx
008E60F7  vmovups ymmword ptr [rax], ymm0
008E60FB  mov     [r14+158h], rax
008E6102  call    sub_10DE1C0
008E6107  mov     rdi, [r14+158h]
008E610E  mov     esi, 6Dh ; 'm'
008E6113  call    sub_10DEA40
008E6118  mov     rax, cs:qword_19A9B88
008E611F  mov     rax, [rax]
008E6122  cmp     rax, [rbp-30h]
008E6126  jnz     short loc_8E6137
008E6128  add     rsp, 38h
008E612C  pop     rbx
008E612D  pop     r12
008E612F  pop     r13
008E6131  pop     r14
008E6133  pop     r15
008E6135  pop     rbp
008E6136  retn
008E6137  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E613C  align 20h
008E6140  push    rbp
008E6141  mov     rbp, rsp
008E6144  push    r15
008E6146  push    r14
008E6148  push    r13
008E614A  push    r12
008E614C  push    rbx
008E614D  sub     rsp, 98h
008E6154  mov     rax, cs:qword_19A9B88
008E615B  mov     [rbp-0B0h], rdx
008E6162  mov     r12, rdi
008E6165  mov     r15, rsi
008E6168  lea     rbx, [r12+40h]
008E616D  mov     rax, [rax]
008E6170  mov     [rbp-30h], rax
008E6174  mov     r14d, [rdx+14h]
008E6178  call    sub_10DEF20
008E617D  mov     rdi, rbx
008E6180  mov     r13d, eax
008E6183  call    sub_8E1820
008E6188  mov     edi, r14d
008E618B  mov     ebx, eax
008E618D  call    sub_8E1790
008E6192  lea     rsi, [rbp-34h]
008E6196  mov     r8d, 1
008E619C  mov     edi, r13d
008E619F  mov     edx, ebx
008E61A1  mov     ecx, eax
008E61A3  call    sub_10F4A00
008E61A8  lea     rbx, [rbp-68h]
008E61AC  mov     rdi, rbx
008E61AF  call    sub_10DC780
008E61B4  mov     dword ptr [rbp-68h], 0Dh
008E61BB  mov     edi, r14d
008E61BE  mov     eax, [r12+70h]
008E61C3  mov     [rbp-64h], eax
008E61C6  mov     eax, [r12+74h]
008E61CB  mov     [rbp-60h], eax
008E61CE  mov     dword ptr [rbp-5Ch], 1
008E61D5  mov     dword ptr [rbp-58h], 0
008E61DC  mov     rax, [r15+98h]
008E61E3  mov     [rbp-0B8h], r15
008E61EA  sub     rax, [r15+90h]
008E61F1  shr     rax, 4
008E61F5  imul    eax, 0CCCCCCCDh
008E61FB  mov     [rbp-50h], eax
008E61FE  call    sub_8E1790
008E6203  mov     rdi, [rbp-0B0h]
008E620A  mov     ecx, [rbp-34h]
008E620D  mov     [rbp-4Ch], eax
008E6210  mov     [rbp-48h], ecx
008E6213  mov     dword ptr [rbp-40h], 0
008E621A  mov     r15, rdi
008E621D  call    sub_6832D0
008E6222  inc     eax
008E6224  mov     edi, 20h ; ' '
008E6229  mov     [rbp-54h], eax
008E622C  mov     [rbp-44h], r13d
008E6230  call    sub_37BF40
008E6235  vxorps  ymm0, ymm0, ymm0
008E6239  mov     rdi, rax
008E623C  mov     rsi, rbx
008E623F  vmovups ymmword ptr [rax], ymm0
008E6243  mov     [r12+158h], rax
008E624B  call    sub_10DC7A0
008E6250  mov     rdi, [r12+158h]
008E6258  call    sub_10DCD20
008E625D  mov     cl, cs:byte_1ADF260
008E6263  mov     r14, rax
008E6266  mov     rbx, r14
008E6269  shr     rbx, 20h
008E626D  test    cl, cl
008E626F  jnz     short loc_8E62A0
008E6271  lea     rdi, byte_1ADF260
008E6278  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E627D  test    eax, eax
008E627F  jz      short loc_8E62A0
008E6281  lea     rdi, aGpu_11; "gpu"
008E6288  call    sub_37BA70
008E628D  lea     rdi, byte_1ADF260
008E6294  mov     cs:qword_1ADF258, rax
008E629B  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E62A0  mov     rdi, cs:qword_1ADF258
008E62A7  call    sub_37A920
008E62AC  mov     rsi, [r12+98h]
008E62B4  mov     edi, r14d
008E62B7  mov     edx, ebx
008E62B9  call    sub_37AE70
008E62BE  mov     [rbp-0C0h], r12
008E62C5  mov     [r12+160h], rax
008E62CD  call    sub_37A9B0
008E62D2  mov     r13, r15
008E62D5  cmp     qword ptr [r13+18h], 0
008E62DA  jz      loc_8E63D6
008E62E0  mov     rax, [rbp-0B8h]
008E62E7  mov     r14, [rax+90h]
008E62EE  cmp     [rax+98h], r14
008E62F5  jz      loc_8E63D6
008E62FB  xor     ebx, ebx
008E62FD  nop     dword ptr [rax]
008E6300  mov     rdi, r13
008E6303  call    sub_6832D0
008E6308  mov     r13, [rbp-0C0h]
008E630F  cmp     rax, 0FFFFFFFFFFFFFFFFh
008E6313  lea     r12, [rbp-98h]
008E631A  jz      short loc_8E6399
008E631C  lea     rax, [rbx+rbx*4]
008E6320  xor     r15d, r15d
008E6323  shl     rax, 4
008E6327  add     r14, rax
008E632A  nop     word ptr [rax+rax+00h]
008E6330  mov     rsi, [r13+158h]
008E6337  mov     rdi, r12
008E633A  mov     edx, r15d
008E633D  mov     ecx, ebx
008E633F  call    sub_10FC210
008E6344  mov     rdx, [r13+158h]
008E634B  lea     rdi, [rbp-0A0h]
008E6352  lea     rsi, [rbp-0A8h]
008E6359  mov     ecx, r15d
008E635C  mov     r8d, ebx
008E635F  call    sub_10FD200
008E6364  mov     rdi, [r13+160h]
008E636B  mov     rsi, [r14+18h]
008E636F  mov     rdx, r12
008E6372  add     rdi, [rbp-0A0h]
008E6379  call    sub_10FDB10
008E637E  mov     rdi, [rbp-0B0h]
008E6385  mov     r14, [r14+28h]
008E6389  inc     r15
008E638C  call    sub_6832D0
008E6391  inc     rax
008E6394  cmp     r15, rax
008E6397  jb      short loc_8E6330
008E6399  mov     rax, [rbp-0B8h]
008E63A0  mov     rcx, 0CCCCCCCCCCCCCCCDh
008E63AA  mov     r13, [rbp-0B0h]
008E63B1  inc     rbx
008E63B4  mov     r14, [rax+90h]
008E63BB  mov     rax, [rax+98h]
008E63C2  sub     rax, r14
008E63C5  sar     rax, 4
008E63C9  imul    rax, rcx
008E63CD  cmp     rbx, rax
008E63D0  jb      loc_8E6300
008E63D6  mov     rbx, [rbp-0C0h]
008E63DD  mov     rsi, [rbx+160h]
008E63E4  mov     rdi, [rbx+158h]
008E63EB  shr     rsi, 8
008E63EF  call    sub_10DDED0
008E63F4  test    byte ptr [rbx+68h], 2
008E63F8  jnz     short loc_8E6408
008E63FA  mov     rdi, [rbx+158h]
008E6401  mov     esi, 10h
008E6406  jmp     short loc_8E6461
008E6408  mov     edi, 40h ; '@'
008E640D  call    sub_37BF40
008E6412  vxorps  ymm0, ymm0, ymm0
008E6416  mov     ecx, 410h
008E641B  xor     edi, edi
008E641D  vmovups ymmword ptr [rax+20h], ymm0
008E6422  vmovups ymmword ptr [rax], ymm0
008E6426  mov     [rbx+178h], rax
008E642D  mov     rsi, [rbx+158h]
008E6434  mov     edx, [rsi+0Ch]
008E6437  bextr   ecx, edx, ecx
008E643C  cmp     edx, 0DFFFFFFFh
008E6442  cmovbe  ecx, edi
008E6445  xor     edx, edx
008E6447  xor     r8d, r8d
008E644A  xor     r9d, r9d
008E644D  mov     rdi, rax
008E6450  call    sub_10DAA70
008E6455  mov     rdi, [rbx+158h]
008E645C  mov     esi, 6Dh ; 'm'
008E6461  call    sub_10DEA40
008E6466  mov     rax, cs:qword_19A9B88
008E646D  mov     rax, [rax]
008E6470  cmp     rax, [rbp-30h]
008E6474  jnz     short loc_8E6488
008E6476  add     rsp, 98h
008E647D  pop     rbx
008E647E  pop     r12
008E6480  pop     r13
008E6482  pop     r14
008E6484  pop     r15
008E6486  pop     rbp
008E6487  retn
008E6488  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E648D  align 10h
