; IDA disassembly evidence for the Orbis 1D texture-array backend.
; Range: 0x8E58A0-0x8E5C1F

008E58A0  push    rbp; Orbis 1D texture-array destructor; defers GPU storage, releases the Gnm descriptor, then runs the common destructor.
008E58A1  mov     rbp, rsp
008E58A4  push    rbx
008E58A5  push    rax
008E58A6  lea     rax, unk_195FAD0
008E58AD  lea     rcx, g_orbis_render_system
008E58B4  mov     rbx, rdi
008E58B7  add     rax, 10h
008E58BB  mov     [rbx], rax
008E58BE  mov     rdi, [rcx]
008E58C1  mov     rsi, [rbx+160h]
008E58C8  call    orbis_defer_allocation_release
008E58CD  mov     rdi, [rbx+158h]
008E58D4  test    rdi, rdi
008E58D7  jz      short loc_8E58DE
008E58D9  call    sub_37BF50
008E58DE  mov     qword ptr [rbx+158h], 0
008E58E9  mov     rdi, rbx
008E58EC  add     rsp, 8
008E58F0  pop     rbx
008E58F1  pop     rbp
008E58F2  jmp     sub_697020
008E58F7  align 20h
008E5900  push    rbp; Deleting destructor for the 360-byte OrbisTextureArray1D object.
008E5901  mov     rbp, rsp
008E5904  push    rbx
008E5905  push    rax
008E5906  lea     rax, unk_195FAD0
008E590D  lea     rcx, g_orbis_render_system
008E5914  mov     rbx, rdi
008E5917  add     rax, 10h
008E591B  mov     [rbx], rax
008E591E  mov     rdi, [rcx]
008E5921  mov     rsi, [rbx+160h]
008E5928  call    orbis_defer_allocation_release
008E592D  mov     rdi, [rbx+158h]
008E5934  test    rdi, rdi
008E5937  jz      short loc_8E593E
008E5939  call    sub_37BF50
008E593E  mov     rdi, rbx
008E5941  mov     qword ptr [rbx+158h], 0
008E594C  call    sub_697020
008E5951  mov     rdi, rbx
008E5954  add     rsp, 8
008E5958  pop     rbx
008E5959  pop     rbp
008E595A  jmp     sub_37BF50
008E595F  align 20h
008E5960  push    rbp; Creates a Gnm 1D-array texture, allocates tiled storage, and uploads every mip of every 80-byte layer record.
008E5961  mov     rbp, rsp
008E5964  push    r15
008E5966  push    r14
008E5968  push    r13
008E596A  push    r12
008E596C  push    rbx
008E596D  sub     rsp, 88h
008E5974  mov     rax, cs:qword_19A9B88
008E597B  mov     r15, rdi
008E597E  lea     rbx, [r15+40h]
008E5982  mov     rax, [rax]
008E5985  mov     [rbp-30h], rax
008E5989  mov     r14, [r15+138h]
008E5990  mov     r13d, [r14+14h]
008E5994  call    sub_10DEF20
008E5999  mov     rdi, rbx
008E599C  mov     r12d, eax
008E599F  call    sub_8E1820
008E59A4  mov     edi, r13d
008E59A7  mov     ebx, eax
008E59A9  call    sub_8E1790
008E59AE  lea     rsi, [rbp-34h]
008E59B2  mov     r8d, 1
008E59B8  mov     edi, r12d
008E59BB  mov     edx, ebx
008E59BD  mov     ecx, eax
008E59BF  call    sub_10F4A00
008E59C4  lea     rbx, [rbp-68h]
008E59C8  mov     rdi, rbx
008E59CB  call    sub_10DC780
008E59D0  mov     dword ptr [rbp-68h], 0Ch
008E59D7  mov     edi, r13d
008E59DA  mov     r13, r14
008E59DD  mov     eax, [r15+70h]
008E59E1  mov     [rbp-64h], eax
008E59E4  mov     dword ptr [rbp-60h], 1
008E59EB  mov     dword ptr [rbp-5Ch], 1
008E59F2  mov     dword ptr [rbp-58h], 0
008E59F9  mov     rax, [r15+140h]
008E5A00  sub     rax, [r15+138h]
008E5A07  shr     rax, 4
008E5A0B  imul    eax, 0CCCCCCCDh
008E5A11  mov     [rbp-50h], eax
008E5A14  call    sub_8E1790
008E5A19  mov     ecx, [rbp-34h]
008E5A1C  mov     rdi, r13
008E5A1F  mov     [rbp-4Ch], eax
008E5A22  mov     [rbp-48h], ecx
008E5A25  mov     dword ptr [rbp-40h], 0
008E5A2C  call    sub_6832D0
008E5A31  inc     eax
008E5A33  mov     edi, 20h ; ' '
008E5A38  mov     [rbp-54h], eax
008E5A3B  mov     [rbp-44h], r12d
008E5A3F  call    sub_37BF40
008E5A44  vxorps  ymm0, ymm0, ymm0
008E5A48  mov     rdi, rax
008E5A4B  mov     rsi, rbx
008E5A4E  vmovups ymmword ptr [rax], ymm0
008E5A52  mov     [r15+158h], rax
008E5A59  call    sub_10DC7A0
008E5A5E  mov     rdi, [r15+158h]
008E5A65  call    sub_10DCD20
008E5A6A  mov     cl, cs:byte_1ADF240
008E5A70  mov     r14, rax
008E5A73  mov     rbx, r14
008E5A76  shr     rbx, 20h
008E5A7A  test    cl, cl
008E5A7C  jnz     short loc_8E5AAD
008E5A7E  lea     rdi, byte_1ADF240
008E5A85  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E5A8A  test    eax, eax
008E5A8C  jz      short loc_8E5AAD
008E5A8E  lea     rdi, aGpu_10; "gpu"
008E5A95  call    sub_37BA70
008E5A9A  lea     rdi, byte_1ADF240
008E5AA1  mov     cs:qword_1ADF238, rax
008E5AA8  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E5AAD  mov     rdi, cs:qword_1ADF238
008E5AB4  call    sub_37A920
008E5AB9  mov     rsi, [r15+98h]
008E5AC0  mov     edi, r14d
008E5AC3  mov     edx, ebx
008E5AC5  call    sub_37AE70
008E5ACA  mov     [r15+160h], rax
008E5AD1  call    sub_37A9B0
008E5AD6  cmp     qword ptr [r13+18h], 0
008E5ADB  jz      loc_8E5BCF
008E5AE1  mov     r14, [r15+138h]
008E5AE8  cmp     [r15+140h], r14
008E5AEF  jz      loc_8E5BCF
008E5AF5  xor     ebx, ebx
008E5AF7  mov     [rbp-0B0h], r13
008E5AFE  xchg    ax, ax
008E5B00  mov     rdi, r13
008E5B03  call    sub_6832D0
008E5B08  cmp     rax, 0FFFFFFFFFFFFFFFFh
008E5B0C  lea     r12, [rbp-98h]
008E5B13  jz      loc_8E5B99
008E5B19  lea     rax, [rbx+rbx*4]
008E5B1D  xor     r13d, r13d
008E5B20  shl     rax, 4
008E5B24  add     r14, rax
008E5B27  nop     word ptr [rax+rax+00000000h]
008E5B30  mov     rsi, [r15+158h]
008E5B37  mov     rdi, r12
008E5B3A  mov     edx, r13d
008E5B3D  mov     ecx, ebx
008E5B3F  call    sub_10FC210
008E5B44  mov     rdx, [r15+158h]
008E5B4B  lea     rdi, [rbp-0A0h]
008E5B52  lea     rsi, [rbp-0A8h]
008E5B59  mov     ecx, r13d
008E5B5C  mov     r8d, ebx
008E5B5F  call    sub_10FD200
008E5B64  mov     rdi, [r15+160h]
008E5B6B  mov     rsi, [r14+18h]
008E5B6F  mov     rdx, r12
008E5B72  add     rdi, [rbp-0A0h]
008E5B79  call    sub_10FDB10
008E5B7E  mov     rdi, [rbp-0B0h]
008E5B85  mov     r14, [r14+28h]
008E5B89  inc     r13
008E5B8C  call    sub_6832D0
008E5B91  inc     rax
008E5B94  cmp     r13, rax
008E5B97  jb      short loc_8E5B30
008E5B99  mov     r14, [r15+138h]
008E5BA0  mov     rax, [r15+140h]
008E5BA7  mov     rcx, 0CCCCCCCCCCCCCCCDh
008E5BB1  mov     r13, [rbp-0B0h]
008E5BB8  inc     rbx
008E5BBB  sub     rax, r14
008E5BBE  sar     rax, 4
008E5BC2  imul    rax, rcx
008E5BC6  cmp     rbx, rax
008E5BC9  jb      loc_8E5B00
008E5BCF  mov     rsi, [r15+160h]
008E5BD6  mov     rdi, [r15+158h]
008E5BDD  shr     rsi, 8
008E5BE1  call    sub_10DDED0
008E5BE6  mov     rdi, [r15+158h]
008E5BED  mov     esi, 10h
008E5BF2  call    sub_10DEA40
008E5BF7  mov     rax, cs:qword_19A9B88
008E5BFE  mov     rax, [rax]
008E5C01  cmp     rax, [rbp-30h]
008E5C05  jnz     short loc_8E5C19
008E5C07  add     rsp, 88h
008E5C0E  pop     rbx
008E5C0F  pop     r12
008E5C11  pop     r13
008E5C13  pop     r14
008E5C15  pop     r15
008E5C17  pop     rbp
008E5C18  retn
008E5C19  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E5C1E  align 20h
