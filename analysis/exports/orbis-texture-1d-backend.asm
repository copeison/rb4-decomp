; IDA disassembly evidence for the Orbis 1D texture backend.
; Range: 0x8E4F90-0x8E529F

008E4F90  push    rbp; Orbis 1D texture destructor; defers the GPU allocation, releases the Gnm descriptor, then runs the common destructor.
008E4F91  mov     rbp, rsp
008E4F94  push    rbx
008E4F95  push    rax
008E4F96  lea     rax, unk_195F960
008E4F9D  lea     rcx, g_orbis_render_system
008E4FA4  mov     rbx, rdi
008E4FA7  add     rax, 10h
008E4FAB  mov     [rbx], rax
008E4FAE  mov     rdi, [rcx]
008E4FB1  mov     rsi, [rbx+190h]
008E4FB8  call    orbis_defer_allocation_release
008E4FBD  mov     rdi, [rbx+188h]
008E4FC4  test    rdi, rdi
008E4FC7  jz      short loc_8E4FCE
008E4FC9  call    sub_37BF50
008E4FCE  mov     qword ptr [rbx+188h], 0
008E4FD9  mov     rdi, rbx
008E4FDC  add     rsp, 8
008E4FE0  pop     rbx
008E4FE1  pop     rbp
008E4FE2  jmp     sub_6F5970
008E4FE7  align 10h
008E4FF0  push    rbp; Deleting destructor for the 408-byte OrbisTexture1D object.
008E4FF1  mov     rbp, rsp
008E4FF4  push    rbx
008E4FF5  push    rax
008E4FF6  lea     rax, unk_195F960
008E4FFD  lea     rcx, g_orbis_render_system
008E5004  mov     rbx, rdi
008E5007  add     rax, 10h
008E500B  mov     [rbx], rax
008E500E  mov     rdi, [rcx]
008E5011  mov     rsi, [rbx+190h]
008E5018  call    orbis_defer_allocation_release
008E501D  mov     rdi, [rbx+188h]
008E5024  test    rdi, rdi
008E5027  jz      short loc_8E502E
008E5029  call    sub_37BF50
008E502E  mov     rdi, rbx
008E5031  mov     qword ptr [rbx+188h], 0
008E503C  call    sub_6F5970
008E5041  mov     rdi, rbx
008E5044  add     rsp, 8
008E5048  pop     rbx
008E5049  pop     rbp
008E504A  jmp     sub_37BF50
008E504F  align 10h
008E5050  push    rbp; Creates the Gnm 1D texture, allocates aligned tiled storage, uploads every CPU mip, and sets memory type 16.
008E5051  mov     rbp, rsp
008E5054  push    r15
008E5056  push    r14
008E5058  push    r13
008E505A  push    r12
008E505C  push    rbx
008E505D  sub     rsp, 88h
008E5064  mov     rax, cs:qword_19A9B88
008E506B  mov     r15, rdi
008E506E  lea     r14, [r15+138h]
008E5075  lea     rbx, [r15+40h]
008E5079  mov     rax, [rax]
008E507C  mov     [rbp-30h], rax
008E5080  mov     r13d, [r15+14Ch]
008E5087  call    sub_10DEF20
008E508C  mov     rdi, rbx
008E508F  mov     r12d, eax
008E5092  call    sub_8E1820
008E5097  mov     edi, r13d
008E509A  mov     ebx, eax
008E509C  call    sub_8E1790
008E50A1  lea     rsi, [rbp-34h]
008E50A5  mov     r8d, 1
008E50AB  mov     edi, r12d
008E50AE  mov     edx, ebx
008E50B0  mov     ecx, eax
008E50B2  call    sub_10F4A00
008E50B7  lea     rbx, [rbp-68h]
008E50BB  mov     rdi, rbx
008E50BE  call    sub_10DC780
008E50C3  mov     dword ptr [rbp-68h], 8
008E50CA  mov     edi, r13d
008E50CD  mov     eax, [r15+70h]
008E50D1  mov     [rbp-64h], eax
008E50D4  mov     dword ptr [rbp-60h], 1
008E50DB  mov     dword ptr [rbp-5Ch], 1
008E50E2  mov     dword ptr [rbp-58h], 0
008E50E9  mov     dword ptr [rbp-50h], 1
008E50F0  call    sub_8E1790
008E50F5  mov     ecx, [rbp-34h]
008E50F8  mov     rdi, r14
008E50FB  mov     [rbp-4Ch], eax
008E50FE  mov     r13, r14
008E5101  mov     [rbp-48h], ecx
008E5104  mov     dword ptr [rbp-40h], 0
008E510B  call    sub_6832D0
008E5110  inc     eax
008E5112  mov     edi, 20h ; ' '
008E5117  mov     [rbp-54h], eax
008E511A  mov     [rbp-44h], r12d
008E511E  call    sub_37BF40
008E5123  vxorps  ymm0, ymm0, ymm0
008E5127  mov     rdi, rax
008E512A  mov     rsi, rbx
008E512D  vmovups ymmword ptr [rax], ymm0
008E5131  mov     [r15+188h], rax
008E5138  call    sub_10DC7A0
008E513D  mov     rdi, [r15+188h]
008E5144  call    sub_10DCD20
008E5149  mov     cl, cs:byte_1ADF200
008E514F  mov     r14, rax
008E5152  mov     rbx, r14
008E5155  shr     rbx, 20h
008E5159  test    cl, cl
008E515B  jnz     short loc_8E518C
008E515D  lea     rdi, byte_1ADF200
008E5164  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E5169  test    eax, eax
008E516B  jz      short loc_8E518C
008E516D  lea     rdi, aGpu_8; "gpu"
008E5174  call    sub_37BA70
008E5179  lea     rdi, byte_1ADF200
008E5180  mov     cs:qword_1ADF1F8, rax
008E5187  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E518C  mov     rdi, cs:qword_1ADF1F8
008E5193  call    sub_37A920
008E5198  mov     rsi, [r15+98h]
008E519F  mov     edi, r14d
008E51A2  mov     edx, ebx
008E51A4  call    sub_37AE70
008E51A9  mov     [r15+190h], rax
008E51B0  call    sub_37A9B0
008E51B5  cmp     qword ptr [r15+150h], 0
008E51BD  mov     r14, r13
008E51C0  jz      loc_8E5243
008E51C6  mov     rdi, r14
008E51C9  call    sub_6832D0
008E51CE  cmp     rax, 0FFFFFFFFFFFFFFFFh
008E51D2  jz      short loc_8E5243
008E51D4  xor     ebx, ebx
008E51D6  lea     r12, [rbp-98h]
008E51DD  mov     r13, r14
008E51E0  mov     rsi, [r15+188h]
008E51E7  xor     ecx, ecx
008E51E9  mov     rdi, r12
008E51EC  mov     edx, ebx
008E51EE  call    sub_10FC210
008E51F3  mov     rdx, [r15+188h]
008E51FA  xor     r8d, r8d
008E51FD  lea     rdi, [rbp-0A0h]
008E5204  lea     rsi, [rbp-0A8h]
008E520B  mov     ecx, ebx
008E520D  call    sub_10FD200
008E5212  mov     rdi, [r15+190h]
008E5219  mov     rsi, [r13+18h]
008E521D  mov     rdx, r12
008E5220  add     rdi, [rbp-0A0h]
008E5227  call    sub_10FDB10
008E522C  mov     r13, [r13+28h]
008E5230  mov     rdi, r14
008E5233  inc     rbx
008E5236  call    sub_6832D0
008E523B  inc     rax
008E523E  cmp     rbx, rax
008E5241  jb      short loc_8E51E0
008E5243  mov     rsi, [r15+190h]
008E524A  mov     rdi, [r15+188h]
008E5251  shr     rsi, 8
008E5255  call    sub_10DDED0
008E525A  mov     rdi, [r15+188h]
008E5261  mov     esi, 10h
008E5266  call    sub_10DEA40
008E526B  mov     rax, cs:qword_19A9B88
008E5272  mov     rax, [rax]
008E5275  cmp     rax, [rbp-30h]
008E5279  jnz     short loc_8E528D
008E527B  add     rsp, 88h
008E5282  pop     rbx
008E5283  pop     r12
008E5285  pop     r13
008E5287  pop     r14
008E5289  pop     r15
008E528B  pop     rbp
008E528C  retn
008E528D  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E5292  align 20h
