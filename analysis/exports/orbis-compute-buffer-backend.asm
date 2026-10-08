; IDA disassembly evidence for the Orbis compute-buffer backend.
; Range: 0x8E3290-0x8E37D0

008E3290  push    rbp; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E3291  mov     rbp, rsp
008E3294  push    r14
008E3296  push    rbx
008E3297  lea     rax, unk_195F7A8
008E329E  lea     r14, g_orbis_render_system
008E32A5  mov     rbx, rdi
008E32A8  add     rax, 10h
008E32AC  mov     [rbx], rax
008E32AF  mov     rdi, [r14]
008E32B2  mov     rsi, [rbx+70h]
008E32B6  call    orbis_defer_allocation_release
008E32BB  mov     qword ptr [rbx+70h], 0
008E32C3  mov     rdi, [r14]
008E32C6  mov     rsi, [rbx+78h]
008E32CA  call    orbis_defer_allocation_release
008E32CF  mov     qword ptr [rbx+78h], 0
008E32D7  mov     rdi, rbx
008E32DA  pop     rbx
008E32DB  pop     r14
008E32DD  pop     rbp
008E32DE  jmp     sub_636D10
008E32E3  align 10h
008E32F0  push    rbp; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E32F1  mov     rbp, rsp
008E32F4  push    r14
008E32F6  push    rbx
008E32F7  lea     rax, unk_195F7A8
008E32FE  lea     r14, g_orbis_render_system
008E3305  mov     rbx, rdi
008E3308  add     rax, 10h
008E330C  mov     [rbx], rax
008E330F  mov     rdi, [r14]
008E3312  mov     rsi, [rbx+70h]
008E3316  call    orbis_defer_allocation_release
008E331B  mov     qword ptr [rbx+70h], 0
008E3323  mov     rdi, [r14]
008E3326  mov     rsi, [rbx+78h]
008E332A  call    orbis_defer_allocation_release
008E332F  mov     rdi, rbx
008E3332  mov     qword ptr [rbx+78h], 0
008E333A  call    sub_636D10
008E333F  mov     rdi, rbx
008E3342  pop     rbx
008E3343  pop     r14
008E3345  pop     rbp
008E3346  jmp     sub_37BF50
008E334B  align 10h
008E3350  push    rbp; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E3351  mov     rbp, rsp
008E3354  push    r15
008E3356  push    r14
008E3358  push    r13
008E335A  push    r12
008E335C  push    rbx
008E335D  push    rax
008E335E  lea     rbx, g_orbis_render_system
008E3365  mov     r14, rdi
008E3368  mov     rsi, [r14+70h]
008E336C  lea     r15, [r14+70h]
008E3370  mov     rdi, [rbx]
008E3373  call    orbis_defer_allocation_release
008E3378  mov     qword ptr [r14+70h], 0
008E3380  mov     rdi, [rbx]
008E3383  mov     rsi, [r14+78h]
008E3387  call    orbis_defer_allocation_release
008E338C  mov     qword ptr [r14+78h], 0
008E3394  lea     rbx, [r14+50h]
008E3398  mov     r13d, [r14+34h]
008E339C  and     r13d, 10h
008E33A0  shr     r13, 4
008E33A4  inc     r13
008E33A7  nop     word ptr [rax+rax+00000000h]
008E33B0  mov     r12, [r14+28h]
008E33B4  test    r12, r12
008E33B7  jnz     loc_8E3470
008E33BD  mov     r12d, [r14+34h]
008E33C1  mov     al, cs:byte_1ADF148
008E33C7  shr     r12d, 1
008E33CA  and     r12d, 4
008E33CE  add     r12d, 4
008E33D2  test    al, al
008E33D4  jnz     short loc_8E3405
008E33D6  lea     rdi, byte_1ADF148
008E33DD  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E33E2  test    eax, eax
008E33E4  jz      short loc_8E3405
008E33E6  lea     rdi, aGpu_4; "gpu"
008E33ED  call    sub_37BA70
008E33F2  lea     rdi, byte_1ADF148
008E33F9  mov     cs:qword_1ADF140, rax
008E3400  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E3405  mov     rdi, cs:qword_1ADF140
008E340C  call    sub_37A920
008E3411  mov     rdi, [r14+10h]
008E3415  lea     rsi, aComputebuffer; "ComputeBuffer"
008E341C  mov     edx, r12d
008E341F  imul    rdi, [r14+18h]
008E3424  call    sub_37AE70
008E3429  mov     r12, rax
008E342C  mov     [r15], r12
008E342F  call    sub_37A9B0
008E3434  mov     rsi, [r14+20h]
008E3438  test    rsi, rsi
008E343B  jz      short loc_8E3450
008E343D  mov     rdx, [r14+10h]
008E3441  mov     rdi, [r15]
008E3444  imul    rdx, [r14+18h]
008E3449  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E344E  jmp     short loc_8E3470
008E3450  test    byte ptr [r14+34h], 8
008E3455  jz      short loc_8E3470
008E3457  mov     rax, [r15]
008E345A  vmovups xmm0, cs:xmmword_12D1730
008E3462  vmovups xmmword ptr [rax], xmm0
008E3466  nop     word ptr [rax+rax+00000000h]
008E3470  mov     edx, [r14+10h]
008E3474  mov     ecx, [r14+18h]
008E3478  mov     rdi, rbx
008E347B  mov     rsi, r12
008E347E  call    sub_10C1FF0
008E3483  test    byte ptr [r14+34h], 9
008E3488  jz      short loc_8E34A0
008E348A  mov     esi, 6Dh ; 'm'
008E348F  jmp     short loc_8E34A5
008E3491  align 20h
008E34A0  mov     esi, 10h
008E34A5  mov     rdi, rbx
008E34A8  call    gnm_buffer_set_resource_memory_type
008E34AD  add     r15, 8
008E34B1  add     rbx, 10h
008E34B5  dec     r13
008E34B8  jnz     loc_8E33B0
008E34BE  mov     al, 1
008E34C0  add     rsp, 8
008E34C4  pop     rbx
008E34C5  pop     r12
008E34C7  pop     r13
008E34C9  pop     r14
008E34CB  pop     r15
008E34CD  pop     rbp
008E34CE  retn
008E34CF  align 10h
008E34D0  push    rbp; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E34D1  mov     rbp, rsp
008E34D4  push    r14
008E34D6  push    rbx
008E34D7  lea     r14, g_orbis_render_system
008E34DE  mov     rbx, rdi
008E34E1  mov     rsi, [rbx+70h]
008E34E5  mov     rdi, [r14]
008E34E8  call    orbis_defer_allocation_release
008E34ED  mov     qword ptr [rbx+70h], 0
008E34F5  mov     rdi, [r14]
008E34F8  mov     rsi, [rbx+78h]
008E34FC  call    orbis_defer_allocation_release
008E3501  mov     qword ptr [rbx+78h], 0
008E3509  pop     rbx
008E350A  pop     r14
008E350C  pop     rbp
008E350D  retn
008E350E  align 10h
008E3510  mov     eax, [rdi+80h]; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E3516  not     eax
008E3518  and     eax, 1
008E351B  mov     [rdi+80h], rax
008E3522  mov     rax, [rdi+rax*8+70h]
008E3527  mov     rsi, [rdi+40h]
008E352B  mov     rdx, [rdi+48h]
008E352F  mov     rdi, rax
008E3532  jmp     memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E3537  align 20h
008E3540  retn
008E3541  align 10h
008E3550  mov     rax, [rdi+80h]
008E3557  mov     rdx, [rdi+rax*8+70h]
008E355C  imul    rax, [rsi+40D90h], 0E888h
008E3567  lea     rdi, [rsi+rax+5728h]
008E356F  xor     esi, esi
008E3571  jmp     loc_10C7010
008E3576  align 20h
008E3580  push    rbp; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E3581  mov     rbp, rsp
008E3584  push    r15
008E3586  push    r14
008E3588  push    rbx
008E3589  push    rax
008E358A  mov     rbx, rdi
008E358D  mov     r15, rsi
008E3590  mov     esi, 4
008E3595  mov     r14, rdx
008E3598  mov     rax, [rbx+80h]
008E359F  shl     rax, 4
008E35A3  lea     rcx, [rbx+rax+50h]
008E35A8  imul    rax, [r15+40D90h], 0E888h
008E35B3  lea     rdi, [r15+rax+5A10h]
008E35BB  call    sub_10E3600
008E35C0  mov     rax, [rbx+80h]
008E35C7  mov     esi, 2
008E35CC  mov     edx, r14d
008E35CF  shl     rax, 4
008E35D3  lea     rcx, [rbx+rax+50h]
008E35D8  imul    rax, [r15+40D90h], 0E888h
008E35E3  lea     rdi, [r15+rax+5A10h]
008E35EB  add     rsp, 8
008E35EF  pop     rbx
008E35F0  pop     r14
008E35F2  pop     r15
008E35F4  pop     rbp
008E35F5  jmp     sub_10E3600
008E35FA  align 20h
008E3600  mov     rax, [rdi+80h]; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E3607  shl     rax, 4
008E360B  lea     rcx, [rdi+rax+50h]
008E3610  imul    rax, [rsi+40D90h], 0E888h
008E361B  lea     rdi, [rsi+rax+5A10h]
008E3623  mov     esi, 5
008E3628  jmp     sub_10E3600
008E362D  align 10h
008E3630  mov     rax, [rdi+80h]; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E3637  shl     rax, 4
008E363B  lea     rcx, [rdi+rax+50h]
008E3640  imul    rax, [rsi+40D90h], 0E888h
008E364B  lea     rdi, [rsi+rax+5A10h]
008E3653  mov     esi, 6
008E3658  jmp     sub_10E3600
008E365D  align 20h
008E3660  mov     rax, [rdi+80h]; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E3667  shl     rax, 4
008E366B  lea     rcx, [rdi+rax+50h]
008E3670  imul    rax, [rsi+40D90h], 0E888h
008E367B  lea     rdi, [rsi+rax+5A10h]
008E3683  mov     esi, 3
008E3688  jmp     sub_10E3600
008E368D  align 10h
008E3690  mov     r8d, ecx; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E3693  mov     rcx, [rdi+80h]
008E369A  imul    rax, [rsi+40D90h], 0E888h
008E36A5  shl     rcx, 4
008E36A9  test    r8b, 1
008E36AD  lea     rcx, [rdi+rcx+50h]
008E36B2  lea     rdi, [rsi+rax+5A10h]
008E36BA  mov     esi, 1
008E36BF  jnz     short loc_8E36C6
008E36C1  jmp     sub_10E3600
008E36C6  jmp     gnmx_constant_update_engine_set_rw_buffer
008E36CB  align 10h
008E36D0  mov     rax, rdx; Recovered Orbis compute-buffer virtual method; see docs/orbis-compute-buffer.md.
008E36D3  mov     edx, [rsi+4A24h]
008E36D9  test    cl, 1
008E36DC  jnz     short loc_8E3713
008E36DE  cmp     edx, 1
008E36E1  jz      short loc_8E3749
008E36E3  test    edx, edx
008E36E5  jnz     short locret_8E3748
008E36E7  mov     rcx, [rdi+80h]
008E36EE  imul    rdx, [rsi+40D90h], 0E888h
008E36F9  shl     rcx, 4
008E36FD  lea     rcx, [rdi+rcx+50h]
008E3702  lea     rdi, [rsi+rdx+5A10h]
008E370A  xor     esi, esi
008E370C  mov     edx, eax
008E370E  jmp     sub_10E3600
008E3713  cmp     edx, 1
008E3716  jz      short loc_8E3786
008E3718  test    edx, edx
008E371A  jnz     short locret_8E3748
008E371C  mov     rcx, [rdi+80h]
008E3723  imul    rdx, [rsi+40D90h], 0E888h
008E372E  shl     rcx, 4
008E3732  lea     rcx, [rdi+rcx+50h]
008E3737  lea     rdi, [rsi+rdx+5A10h]
008E373F  xor     esi, esi
008E3741  mov     edx, eax
008E3743  jmp     gnmx_constant_update_engine_set_rw_buffer
008E3748  retn
008E3749  imul    rdx, [rsi+40D90h], 0F1E0h
008E3754  mov     rcx, [rdi+80h]
008E375B  shl     rcx, 4
008E375F  add     rdx, rsi
008E3762  imul    rsi, [rsi+4A28h], 1AE0h
008E376D  lea     rcx, [rdi+rcx+50h]
008E3772  lea     rdi, [rsi+rdx+22A20h]
008E377A  mov     edx, 1
008E377F  mov     esi, eax
008E3781  jmp     loc_10EEE70
008E3786  imul    rdx, [rsi+40D90h], 0F1E0h
008E3791  mov     rcx, [rdi+80h]
008E3798  shl     rcx, 4
008E379C  add     rdx, rsi
008E379F  imul    rsi, [rsi+4A28h], 1AE0h
008E37AA  lea     rcx, [rdi+rcx+50h]
008E37AF  lea     rdi, [rsi+rdx+22A20h]
008E37B7  mov     edx, 1
008E37BC  mov     esi, eax
008E37BE  jmp     loc_10EEF40
008E37C3  align 10h
008E37D0  retn
