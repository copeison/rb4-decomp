; IDA disassembly evidence for the Orbis 3D texture backend.
; Range: 0x8E53F0-0x8E573F

008E53F0  push    rbp; Orbis 3D texture destructor; defers the GPU allocation, releases the Gnm descriptor, then runs the common destructor.
008E53F1  mov     rbp, rsp
008E53F4  push    rbx
008E53F5  push    rax
008E53F6  lea     rax, unk_195FA18
008E53FD  lea     rcx, g_orbis_render_system
008E5404  mov     rbx, rdi
008E5407  add     rax, 10h
008E540B  mov     [rbx], rax
008E540E  mov     rdi, [rcx]
008E5411  mov     rsi, [rbx+190h]
008E5418  call    orbis_defer_allocation_release
008E541D  mov     rdi, [rbx+188h]
008E5424  test    rdi, rdi
008E5427  jz      short loc_8E542E
008E5429  call    sub_37BF50
008E542E  mov     qword ptr [rbx+188h], 0
008E5439  mov     rdi, rbx
008E543C  add     rsp, 8
008E5440  pop     rbx
008E5441  pop     rbp
008E5442  jmp     sub_6F5DB0
008E5447  align 10h
008E5450  push    rbp; Deleting destructor for the 408-byte OrbisTexture3D object.
008E5451  mov     rbp, rsp
008E5454  push    rbx
008E5455  push    rax
008E5456  lea     rax, unk_195FA18
008E545D  lea     rcx, g_orbis_render_system
008E5464  mov     rbx, rdi
008E5467  add     rax, 10h
008E546B  mov     [rbx], rax
008E546E  mov     rdi, [rcx]
008E5471  mov     rsi, [rbx+190h]
008E5478  call    orbis_defer_allocation_release
008E547D  mov     rdi, [rbx+188h]
008E5484  test    rdi, rdi
008E5487  jz      short loc_8E548E
008E5489  call    sub_37BF50
008E548E  mov     rdi, rbx
008E5491  mov     qword ptr [rbx+188h], 0
008E549C  call    sub_6F5DB0
008E54A1  mov     rdi, rbx
008E54A4  add     rsp, 8
008E54A8  pop     rbx
008E54A9  pop     rbp
008E54AA  jmp     sub_37BF50
008E54AF  align 10h
008E54B0  push    rbp; Creates the Gnm 3D texture, reuses compatible storage or allocates it, uploads all CPU mip levels, and sets memory type.
008E54B1  mov     rbp, rsp
008E54B4  push    r15
008E54B6  push    r14
008E54B8  push    r13
008E54BA  push    r12
008E54BC  push    rbx
008E54BD  sub     rsp, 98h
008E54C4  mov     rax, cs:qword_19A9B88
008E54CB  mov     [rbp-0B0h], rsi
008E54D2  mov     r13, rdi
008E54D5  lea     r15, [r13+138h]
008E54DC  lea     rbx, [r13+40h]
008E54E0  mov     rax, [rax]
008E54E3  mov     [rbp-30h], rax
008E54E7  mov     r14d, [r13+14Ch]
008E54EE  call    sub_10DEF20
008E54F3  mov     rdi, rbx
008E54F6  mov     r12d, eax
008E54F9  call    sub_8E1820
008E54FE  mov     edi, r14d
008E5501  mov     ebx, eax
008E5503  call    sub_8E1790
008E5508  lea     rsi, [rbp-34h]
008E550C  mov     r8d, 1
008E5512  mov     edi, r12d
008E5515  mov     edx, ebx
008E5517  mov     ecx, eax
008E5519  call    sub_10F4A00
008E551E  lea     rbx, [rbp-68h]
008E5522  mov     rdi, rbx
008E5525  call    sub_10DC780
008E552A  mov     dword ptr [rbp-68h], 0Ah
008E5531  mov     edi, r14d
008E5534  mov     eax, [r13+70h]
008E5538  mov     [rbp-64h], eax
008E553B  mov     eax, [r13+74h]
008E553F  mov     [rbp-60h], eax
008E5542  mov     eax, [r13+78h]
008E5546  mov     [rbp-5Ch], eax
008E5549  mov     dword ptr [rbp-58h], 0
008E5550  mov     dword ptr [rbp-50h], 1
008E5557  call    sub_8E1790
008E555C  mov     ecx, [rbp-34h]
008E555F  mov     rdi, r15
008E5562  mov     [rbp-4Ch], eax
008E5565  mov     [rbp-0B8h], r15
008E556C  mov     [rbp-48h], ecx
008E556F  mov     dword ptr [rbp-40h], 0
008E5576  call    sub_6832D0
008E557B  inc     eax
008E557D  mov     edi, 20h ; ' '
008E5582  mov     [rbp-54h], eax
008E5585  mov     [rbp-44h], r12d
008E5589  call    sub_37BF40
008E558E  vxorps  ymm0, ymm0, ymm0
008E5592  mov     rdi, rax
008E5595  mov     rsi, rbx
008E5598  vmovups ymmword ptr [rax], ymm0
008E559C  mov     [r13+188h], rax
008E55A3  call    sub_10DC7A0
008E55A8  mov     rdi, [r13+188h]
008E55AF  call    sub_10DCD20
008E55B4  mov     rcx, [rbp-0B0h]
008E55BB  mov     rbx, rax
008E55BE  test    rcx, rcx
008E55C1  jz      short loc_8E55D3
008E55C3  mov     rax, [rcx+190h]
008E55CA  mov     [r13+190h], rax
008E55D1  jmp     short loc_8E563C
008E55D3  mov     al, cs:byte_1ADF220
008E55D9  mov     r14, rbx
008E55DC  shr     r14, 20h
008E55E0  test    al, al
008E55E2  jnz     short loc_8E5613
008E55E4  lea     rdi, byte_1ADF220
008E55EB  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E55F0  test    eax, eax
008E55F2  jz      short loc_8E5613
008E55F4  lea     rdi, aGpu_9; "gpu"
008E55FB  call    sub_37BA70
008E5600  lea     rdi, byte_1ADF220
008E5607  mov     cs:qword_1ADF218, rax
008E560E  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E5613  mov     rdi, cs:qword_1ADF218
008E561A  call    sub_37A920
008E561F  mov     rsi, [r13+98h]
008E5626  mov     edi, ebx
008E5628  mov     edx, r14d
008E562B  call    sub_37AE70
008E5630  mov     [r13+190h], rax
008E5637  call    sub_37A9B0
008E563C  mov     r14, [rbp-0B8h]
008E5643  cmp     qword ptr [r13+150h], 0
008E564B  jz      loc_8E56D5
008E5651  mov     rdi, r14
008E5654  call    sub_6832D0
008E5659  cmp     rax, 0FFFFFFFFFFFFFFFFh
008E565D  jz      short loc_8E56D5
008E565F  xor     ebx, ebx
008E5661  lea     r15, [rbp-98h]
008E5668  mov     r12, r14
008E566B  nop     dword ptr [rax+rax+00h]
008E5670  mov     rsi, [r13+188h]
008E5677  xor     ecx, ecx
008E5679  mov     rdi, r15
008E567C  mov     edx, ebx
008E567E  call    sub_10FC210
008E5683  mov     rdx, [r13+188h]
008E568A  xor     r8d, r8d
008E568D  lea     rdi, [rbp-0A0h]
008E5694  lea     rsi, [rbp-0A8h]
008E569B  mov     ecx, ebx
008E569D  call    sub_10FD200
008E56A2  mov     rdi, [r13+190h]
008E56A9  mov     rsi, [r12+18h]
008E56AE  mov     rdx, r15
008E56B1  add     rdi, [rbp-0A0h]
008E56B8  call    sub_10FDB10
008E56BD  mov     r12, [r12+28h]
008E56C2  mov     rdi, r14
008E56C5  inc     rbx
008E56C8  call    sub_6832D0
008E56CD  inc     rax
008E56D0  cmp     rbx, rax
008E56D3  jb      short loc_8E5670
008E56D5  mov     rsi, [r13+190h]
008E56DC  mov     rdi, [r13+188h]
008E56E3  shr     rsi, 8
008E56E7  call    sub_10DDED0
008E56EC  mov     rdi, [r13+188h]
008E56F3  test    byte ptr [r13+68h], 2
008E56F8  jnz     short loc_8E5701
008E56FA  mov     esi, 10h
008E56FF  jmp     short loc_8E5706
008E5701  mov     esi, 6Dh ; 'm'
008E5706  call    sub_10DEA40
008E570B  mov     rax, cs:qword_19A9B88
008E5712  mov     rax, [rax]
008E5715  cmp     rax, [rbp-30h]
008E5719  jnz     short loc_8E572D
008E571B  add     rsp, 98h
008E5722  pop     rbx
008E5723  pop     r12
008E5725  pop     r13
008E5727  pop     r14
008E5729  pop     r15
008E572B  pop     rbp
008E572C  retn
008E572D  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E5732  align 20h
