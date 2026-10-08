; IDA disassembly evidence for the Orbis cube texture-array backend.
; Range: 0x8E6670-0x8E6A7F

008E6670  push    rbp; Orbis cube-array destructor; defers GPU storage, releases the Gnm descriptor, then runs the common destructor.
008E6671  mov     rbp, rsp
008E6674  push    rbx
008E6675  push    rax
008E6676  lea     rax, unk_195FC40
008E667D  lea     rcx, g_orbis_render_system
008E6684  mov     rbx, rdi
008E6687  add     rax, 10h
008E668B  mov     [rbx], rax
008E668E  mov     rdi, [rcx]
008E6691  mov     rsi, [rbx+160h]
008E6698  call    orbis_defer_allocation_release
008E669D  mov     rdi, [rbx+158h]
008E66A4  test    rdi, rdi
008E66A7  jz      short loc_8E66AE
008E66A9  call    sub_37BF50
008E66AE  mov     qword ptr [rbx+158h], 0
008E66B9  mov     rdi, rbx
008E66BC  add     rsp, 8
008E66C0  pop     rbx
008E66C1  pop     rbp
008E66C2  jmp     sub_69ACD0
008E66C7  align 10h
008E66D0  push    rbp; Deleting destructor for the 360-byte OrbisTextureArrayCube object.
008E66D1  mov     rbp, rsp
008E66D4  push    rbx
008E66D5  push    rax
008E66D6  lea     rax, unk_195FC40
008E66DD  lea     rcx, g_orbis_render_system
008E66E4  mov     rbx, rdi
008E66E7  add     rax, 10h
008E66EB  mov     [rbx], rax
008E66EE  mov     rdi, [rcx]
008E66F1  mov     rsi, [rbx+160h]
008E66F8  call    orbis_defer_allocation_release
008E66FD  mov     rdi, [rbx+158h]
008E6704  test    rdi, rdi
008E6707  jz      short loc_8E670E
008E6709  call    sub_37BF50
008E670E  mov     rdi, rbx
008E6711  mov     qword ptr [rbx+158h], 0
008E671C  call    sub_69ACD0
008E6721  mov     rdi, rbx
008E6724  add     rsp, 8
008E6728  pop     rbx
008E6729  pop     rbp
008E672A  jmp     sub_37BF50
008E672F  align 10h
008E6730  push    rbp; Creates a Gnm cube-array texture and uploads every mip for all six faces of every 480-byte cube record.
008E6731  mov     rbp, rsp
008E6734  push    r15
008E6736  push    r14
008E6738  push    r13
008E673A  push    r12
008E673C  push    rbx
008E673D  sub     rsp, 0A8h
008E6744  mov     rax, cs:qword_19A9B88
008E674B  mov     r14, rdi
008E674E  mov     rax, [rax]
008E6751  mov     [rbp-30h], rax
008E6755  mov     rbx, [r14+138h]
008E675C  mov     r13d, [rbx+14h]
008E6760  call    sub_10DEF20
008E6765  mov     edi, r13d
008E6768  mov     r15d, eax
008E676B  call    sub_8E1790
008E6770  lea     rsi, [rbp-34h]
008E6774  mov     edx, 9
008E6779  mov     r8d, 1
008E677F  mov     edi, r15d
008E6782  mov     ecx, eax
008E6784  call    sub_10F4A00
008E6789  lea     r12, [rbp-68h]
008E678D  mov     rdi, r12
008E6790  call    sub_10DC780
008E6795  mov     dword ptr [rbp-68h], 0Bh
008E679C  mov     edi, r13d
008E679F  mov     eax, [r14+70h]
008E67A3  mov     [rbp-64h], eax
008E67A6  mov     eax, [r14+74h]
008E67AA  mov     [rbp-60h], eax
008E67AD  mov     dword ptr [rbp-5Ch], 1
008E67B4  mov     dword ptr [rbp-58h], 0
008E67BB  mov     rax, [r14+140h]
008E67C2  sub     rax, [r14+138h]
008E67C9  shr     rax, 5
008E67CD  imul    eax, 0EEEEEEEFh
008E67D3  mov     [rbp-50h], eax
008E67D6  call    sub_8E1790
008E67DB  mov     ecx, [rbp-34h]
008E67DE  mov     rdi, rbx
008E67E1  mov     [rbp-4Ch], eax
008E67E4  mov     [rbp-48h], ecx
008E67E7  mov     dword ptr [rbp-40h], 0
008E67EE  call    sub_6832D0
008E67F3  inc     eax
008E67F5  mov     edi, 20h ; ' '
008E67FA  mov     [rbp-54h], eax
008E67FD  mov     [rbp-44h], r15d
008E6801  call    sub_37BF40
008E6806  vxorps  ymm0, ymm0, ymm0
008E680A  mov     rdi, rax
008E680D  mov     rsi, r12
008E6810  vmovups ymmword ptr [rax], ymm0
008E6814  mov     [r14+158h], rax
008E681B  call    sub_10DC7A0
008E6820  mov     rdi, [r14+158h]
008E6827  call    sub_10DCD20
008E682C  mov     cl, cs:byte_1ADF290
008E6832  mov     r12, rax
008E6835  mov     r15, r12
008E6838  shr     r15, 20h
008E683C  test    cl, cl
008E683E  jnz     short loc_8E686F
008E6840  lea     rdi, byte_1ADF290
008E6847  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E684C  test    eax, eax
008E684E  jz      short loc_8E686F
008E6850  lea     rdi, aGpu_12; "gpu"
008E6857  call    sub_37BA70
008E685C  lea     rdi, byte_1ADF290
008E6863  mov     cs:qword_1ADF288, rax
008E686A  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E686F  mov     rdi, cs:qword_1ADF288
008E6876  call    sub_37A920
008E687B  mov     rsi, [r14+98h]
008E6882  mov     edi, r12d
008E6885  mov     edx, r15d
008E6888  call    sub_37AE70
008E688D  mov     [r14+160h], rax
008E6894  call    sub_37A9B0
008E6899  cmp     qword ptr [rbx+18h], 0
008E689E  jz      loc_8E6A2F
008E68A4  mov     r12, [r14+138h]
008E68AB  cmp     [r14+140h], r12
008E68B2  jz      loc_8E6A2F
008E68B8  mov     [rbp-0B0h], r14
008E68BF  xor     ecx, ecx
008E68C1  mov     [rbp-0C8h], rbx
008E68C8  mov     r15, [rbp-0B0h]
008E68CF  nop
008E68D0  lea     rax, [rcx+rcx]
008E68D4  mov     [rbp-0C0h], rcx
008E68DB  lea     rax, [rax+rax*2]
008E68DF  mov     [rbp-0D0h], rax
008E68E6  xor     eax, eax
008E68E8  mov     [rbp-0B8h], rax
008E68EF  jmp     short loc_8E690E
008E68F1  align 20h
008E6900  mov     rax, [rbp-0B0h]
008E6907  mov     r12, [rax+138h]
008E690E  mov     rdi, rbx
008E6911  call    sub_6832D0
008E6916  cmp     rax, 0FFFFFFFFFFFFFFFFh
008E691A  lea     rbx, [rbp-98h]
008E6921  jz      loc_8E69CA
008E6927  imul    rax, [rbp-0C0h], 1E0h
008E6932  mov     rcx, [rbp-0B8h]
008E6939  xor     r13d, r13d
008E693C  lea     r14, [rcx+rcx*4]
008E6940  add     r12, rax
008E6943  mov     rax, [rbp-0D0h]
008E694A  shl     r14, 4
008E694E  add     r14, r12
008E6951  lea     r12d, [rcx+rax]
008E6955  nop     word ptr [rax+rax+00000000h]
008E6960  mov     rsi, [r15+158h]
008E6967  mov     rdi, rbx
008E696A  mov     edx, r13d
008E696D  mov     ecx, r12d
008E6970  call    sub_10FC210
008E6975  mov     rdx, [r15+158h]
008E697C  lea     rdi, [rbp-0A0h]
008E6983  lea     rsi, [rbp-0A8h]
008E698A  mov     ecx, r13d
008E698D  mov     r8d, r12d
008E6990  call    sub_10FD200
008E6995  mov     rdi, [r15+160h]
008E699C  mov     rsi, [r14+18h]
008E69A0  mov     rdx, rbx
008E69A3  add     rdi, [rbp-0A0h]
008E69AA  call    sub_10FDB10
008E69AF  mov     rdi, [rbp-0C8h]
008E69B6  mov     r14, [r14+28h]
008E69BA  inc     r13
008E69BD  call    sub_6832D0
008E69C2  inc     rax
008E69C5  cmp     r13, rax
008E69C8  jb      short loc_8E6960
008E69CA  mov     rax, [rbp-0B8h]
008E69D1  mov     rbx, [rbp-0C8h]
008E69D8  mov     rcx, rax
008E69DB  inc     rcx
008E69DE  mov     rax, rcx
008E69E1  cmp     rcx, 6
008E69E5  mov     [rbp-0B8h], rax
008E69EC  jnz     loc_8E6900
008E69F2  mov     r14, [rbp-0B0h]
008E69F9  mov     rcx, [rbp-0C0h]
008E6A00  mov     rdx, 0EEEEEEEEEEEEEEEFh
008E6A0A  mov     r12, [r14+138h]
008E6A11  mov     rax, [r14+140h]
008E6A18  inc     rcx
008E6A1B  sub     rax, r12
008E6A1E  sar     rax, 5
008E6A22  imul    rax, rdx
008E6A26  cmp     rcx, rax
008E6A29  jb      loc_8E68D0
008E6A2F  mov     rsi, [r14+160h]
008E6A36  mov     rdi, [r14+158h]
008E6A3D  shr     rsi, 8
008E6A41  call    sub_10DDED0
008E6A46  mov     rdi, [r14+158h]
008E6A4D  mov     esi, 10h
008E6A52  call    sub_10DEA40
008E6A57  mov     rax, cs:qword_19A9B88
008E6A5E  mov     rax, [rax]
008E6A61  cmp     rax, [rbp-30h]
008E6A65  jnz     short loc_8E6A79
008E6A67  add     rsp, 0A8h
008E6A6E  pop     rbx
008E6A6F  pop     r12
008E6A71  pop     r13
008E6A73  pop     r14
008E6A75  pop     r15
008E6A77  pop     rbp
008E6A78  retn
008E6A79  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E6A7E  align 20h
