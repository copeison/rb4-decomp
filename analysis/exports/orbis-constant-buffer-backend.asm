; IDA disassembly evidence for the Orbis constant-buffer backend.
; Functions: 0x8E3800-0x8E3CF5

; orbis_constant_buffer_construct at 0x8e3800
008E3800  push    rbp
008E3801  mov     rbp, rsp
008E3804  push    rbx
008E3805  push    rax
008E3806  mov     rbx, rdi
008E3809  call    constant_buffer_construct
008E380E  lea     rax, unk_195F828
008E3815  vxorps  ymm0, ymm0, ymm0
008E3819  add     rax, 10h
008E381D  mov     [rbx], rax
008E3820  vmovups ymmword ptr [rbx+50h], ymm0
008E3825  add     rsp, 8
008E3829  pop     rbx
008E382A  pop     rbp
008E382B  retn

; orbis_constant_buffer_destruct at 0x8e3830
008E3830  push    rbp
008E3831  mov     rbp, rsp
008E3834  push    rbx
008E3835  push    rax
008E3836  lea     rax, unk_195F828
008E383D  mov     rbx, rdi
008E3840  add     rax, 10h
008E3844  mov     [rbx], rax
008E3847  mov     rsi, [rbx+58h]
008E384B  test    rsi, rsi
008E384E  jz      short loc_8E3867
008E3850  lea     rax, g_orbis_render_system
008E3857  mov     rdi, [rax]
008E385A  call    orbis_defer_allocation_release
008E385F  mov     qword ptr [rbx+58h], 0
008E3867  mov     qword ptr [rbx+60h], 0
008E386F  add     rsp, 8
008E3873  pop     rbx
008E3874  pop     rbp
008E3875  retn

; orbis_constant_buffer_release_backend at 0x8e3880
008E3880  push    rbp
008E3881  mov     rbp, rsp
008E3884  push    rbx
008E3885  push    rax
008E3886  mov     rbx, rdi
008E3889  mov     rsi, [rbx+58h]
008E388D  test    rsi, rsi
008E3890  jz      short loc_8E38A9
008E3892  lea     rax, g_orbis_render_system
008E3899  mov     rdi, [rax]
008E389C  call    orbis_defer_allocation_release
008E38A1  mov     qword ptr [rbx+58h], 0
008E38A9  mov     qword ptr [rbx+60h], 0
008E38B1  add     rsp, 8
008E38B5  pop     rbx
008E38B6  pop     rbp
008E38B7  retn

; orbis_constant_buffer_delete at 0x8e38c0
008E38C0  push    rbp
008E38C1  mov     rbp, rsp
008E38C4  push    rbx
008E38C5  push    rax
008E38C6  lea     rax, unk_195F828
008E38CD  mov     rbx, rdi
008E38D0  add     rax, 10h
008E38D4  mov     [rbx], rax
008E38D7  mov     rsi, [rbx+58h]
008E38DB  test    rsi, rsi
008E38DE  jz      short loc_8E38EF
008E38E0  lea     rax, g_orbis_render_system
008E38E7  mov     rdi, [rax]
008E38EA  call    orbis_defer_allocation_release
008E38EF  mov     rdi, rbx
008E38F2  add     rsp, 8
008E38F6  pop     rbx
008E38F7  pop     rbp
008E38F8  jmp     sub_37BF50

; orbis_constant_buffer_initialize_backend at 0x8e3900
008E3900  push    rbp
008E3901  mov     rbp, rsp
008E3904  push    rbx
008E3905  push    rax
008E3906  mov     rbx, rdi
008E3909  mov     rsi, [rbx+58h]
008E390D  test    rsi, rsi
008E3910  jz      short loc_8E3929
008E3912  lea     rax, g_orbis_render_system
008E3919  mov     rdi, [rax]
008E391C  call    orbis_defer_allocation_release
008E3921  mov     qword ptr [rbx+58h], 0
008E3929  mov     rax, [rbx+28h]
008E392D  mov     edi, 10h
008E3932  lea     rsi, aCbuffer_0; "CBuffer"
008E3939  mov     edx, 4
008E393E  mov     rcx, rax
008E3941  shl     rcx, 4
008E3945  test    rax, rax
008E3948  cmovnz  rdi, rcx
008E394C  mov     [rbx+60h], rdi
008E3950  call    sub_37AE70
008E3955  mov     [rbx+58h], rax
008E3959  mov     rdi, rax
008E395C  mov     rsi, [rbx+30h]
008E3960  mov     rdx, [rbx+60h]
008E3964  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E3969  mov     qword ptr [rbx+50h], 0
008E3971  add     rsp, 8
008E3975  pop     rbx
008E3976  pop     rbp
008E3977  retn

; orbis_constant_buffer_update_range at 0x8e3980
008E3980  push    rbp
008E3981  mov     rbp, rsp
008E3984  push    rbx
008E3985  push    rax
008E3986  mov     rbx, rdi
008E3989  sub     rcx, rdx
008E398C  shl     rdx, 4
008E3990  add     rdx, [rbx+58h]
008E3994  mov     rsi, [rbx+30h]
008E3998  shl     rcx, 4
008E399C  mov     rdi, rdx
008E399F  mov     rdx, rcx
008E39A2  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E39A7  mov     qword ptr [rbx+50h], 0
008E39AF  add     rsp, 8
008E39B3  pop     rbx
008E39B4  pop     rbp
008E39B5  retn

; orbis_constant_buffer_bind at 0x8e39c0
008E39C0  push    rbp
008E39C1  mov     rbp, rsp
008E39C4  push    r15
008E39C6  push    r14
008E39C8  push    r13
008E39CA  push    r12
008E39CC  push    rbx
008E39CD  sub     rsp, 38h
008E39D1  mov     r12, cs:qword_19A9B88
008E39D8  mov     rbx, rdi
008E39DB  lea     rcx, g_render_system
008E39E2  mov     r15, rsi
008E39E5  mov     rax, [r12]
008E39E9  mov     [rbp-30h], rax
008E39ED  mov     eax, [rbx+14h]
008E39F0  mov     [rbp-54h], eax
008E39F3  mov     rax, [rcx]
008E39F6  mov     r13d, [rbx+18h]
008E39FA  mov     rax, [rax+0A0h]
008E3A01  cmp     [rbx+68h], rax
008E3A05  jnz     short loc_8E3A24
008E3A07  mov     rdi, [rbx+50h]
008E3A0B  lea     r14, [rbx+50h]
008E3A0F  test    rdi, rdi
008E3A12  jz      short loc_8E3A34
008E3A14  lea     rax, [r15+4A24h]
008E3A1B  mov     [rbp-50h], rax
008E3A1F  jmp     loc_8E3B67
008E3A24  lea     r14, [rbx+50h]
008E3A28  mov     [rbx+68h], rax
008E3A2C  mov     qword ptr [rbx+50h], 0
008E3A34  lea     rax, [r15+4A24h]
008E3A3B  mov     [rbp-50h], rax
008E3A3F  mov     eax, [r15+4A24h]
008E3A46  cmp     eax, 1
008E3A49  jz      loc_8E3AD4
008E3A4F  test    eax, eax
008E3A51  jnz     loc_8E3B57
008E3A57  imul    rax, [r15+40D90h], 0E888h
008E3A62  mov     r12d, [rbx+60h]
008E3A66  add     r12, 3
008E3A6A  mov     rdi, [r15+rax+5730h]
008E3A72  shr     r12, 2
008E3A76  lea     rdx, [r15+rax+5730h]
008E3A7E  lea     esi, [r12+2]
008E3A83  mov     rcx, rdi
008E3A86  sub     rcx, [r15+rax+5738h]
008E3A8E  shr     rcx, 2
008E3A92  cmp     ecx, esi
008E3A94  jnb     short loc_8E3AC1
008E3A96  mov     [rbp-48h], rdx
008E3A9A  lea     rdi, [r15+rax+5728h]
008E3AA2  mov     rdx, [r15+rax+5748h]
008E3AAA  call    qword ptr [r15+rax+5740h]
008E3AB2  test    al, al
008E3AB4  jz      loc_8E3B5B
008E3ABA  mov     rdx, [rbp-48h]
008E3ABE  mov     rdi, [rdx]
008E3AC1  shl     r12, 2
008E3AC5  sub     rdi, r12
008E3AC8  and     rdi, 0FFFFFFFFFFFFFFFCh
008E3ACC  mov     [rdx], rdi
008E3ACF  jmp     loc_8E3B5D
008E3AD4  imul    rcx, [r15+40D90h], 0F1E0h
008E3ADF  imul    rax, [r15+4A28h], 1AE0h
008E3AEA  mov     r12d, [rbx+60h]
008E3AEE  add     r12, 3
008E3AF2  add     rcx, r15
008E3AF5  shr     r12, 2
008E3AF9  mov     rdi, qword ptr ds:sub_229E8[rax+rcx]
008E3B01  lea     esi, [r12+2]
008E3B06  lea     r8, sub_229E8[rax+rcx]
008E3B0E  mov     rdx, rdi
008E3B11  sub     rdx, qword ptr ds:loc_229F0[rax+rcx]
008E3B19  shr     rdx, 2
008E3B1D  cmp     edx, esi
008E3B1F  jnb     short loc_8E3B47
008E3B21  mov     [rbp-48h], r8
008E3B25  lea     rdi, [rax+rcx+229E0h]
008E3B2D  mov     rdx, qword ptr ds:sub_22A00[rax+rcx]
008E3B35  call    ds:qword_229F8[rax+rcx]
008E3B3C  test    al, al
008E3B3E  jz      short loc_8E3B5B
008E3B40  mov     r8, [rbp-48h]
008E3B44  mov     rdi, [r8]
008E3B47  shl     r12, 2
008E3B4B  sub     rdi, r12
008E3B4E  and     rdi, 0FFFFFFFFFFFFFFFCh
008E3B52  mov     [r8], rdi
008E3B55  jmp     short loc_8E3B5D
008E3B57  xor     edi, edi
008E3B59  jmp     short loc_8E3B67
008E3B5B  xor     edi, edi
008E3B5D  mov     r12, cs:qword_19A9B88
008E3B64  mov     [r14], rdi
008E3B67  mov     rsi, [rbx+58h]
008E3B6B  mov     rdx, [rbx+60h]
008E3B6F  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E3B74  mov     edx, [rbx+60h]
008E3B77  mov     rsi, [r14]
008E3B7A  lea     rbx, [rbp-40h]
008E3B7E  mov     rdi, rbx
008E3B81  call    sub_10C20C0
008E3B86  mov     esi, 10h
008E3B8B  mov     rdi, rbx
008E3B8E  call    gnm_buffer_set_resource_memory_type
008E3B93  mov     rbx, [rbp-50h]
008E3B97  mov     r14d, [rbp-54h]
008E3B9B  cmp     dword ptr [rbx], 0
008E3B9E  jnz     loc_8E3C76
008E3BA4  test    r13b, 1
008E3BA8  jz      short loc_8E3BCE
008E3BAA  imul    rax, [r15+40D90h], 0E888h
008E3BB5  lea     rcx, [rbp-40h]
008E3BB9  mov     esi, 2
008E3BBE  mov     edx, r14d
008E3BC1  lea     rdi, [r15+rax+5A10h]
008E3BC9  call    sub_10E40A0
008E3BCE  test    r13b, 2
008E3BD2  jz      short loc_8E3C22
008E3BD4  imul    rax, [r15+40D90h], 0E888h
008E3BDF  lea     rbx, [rbp-40h]
008E3BE3  mov     esi, 5
008E3BE8  mov     edx, r14d
008E3BEB  mov     rcx, rbx
008E3BEE  lea     rdi, [r15+rax+5A10h]
008E3BF6  call    sub_10E40A0
008E3BFB  imul    rax, [r15+40D90h], 0E888h
008E3C06  mov     rcx, rbx
008E3C09  mov     rbx, [rbp-50h]
008E3C0D  mov     esi, 6
008E3C12  mov     edx, r14d
008E3C15  lea     rdi, [r15+rax+5A10h]
008E3C1D  call    sub_10E40A0
008E3C22  test    r13b, 4
008E3C26  jz      short loc_8E3C4C
008E3C28  imul    rax, [r15+40D90h], 0E888h
008E3C33  lea     rcx, [rbp-40h]
008E3C37  mov     esi, 3
008E3C3C  mov     edx, r14d
008E3C3F  lea     rdi, [r15+rax+5A10h]
008E3C47  call    sub_10E40A0
008E3C4C  test    r13b, 8
008E3C50  jz      short loc_8E3C76
008E3C52  imul    rax, [r15+40D90h], 0E888h
008E3C5D  lea     rcx, [rbp-40h]
008E3C61  mov     esi, 1
008E3C66  mov     edx, r14d
008E3C69  lea     rdi, [r15+rax+5A10h]
008E3C71  call    sub_10E40A0
008E3C76  test    r13b, 10h
008E3C7A  jz      short loc_8E3CDC
008E3C7C  mov     eax, [rbx]
008E3C7E  cmp     eax, 1
008E3C81  jz      short loc_8E3CAA
008E3C83  test    eax, eax
008E3C85  jnz     short loc_8E3CDC
008E3C87  imul    rax, [r15+40D90h], 0E888h
008E3C92  lea     rcx, [rbp-40h]
008E3C96  xor     esi, esi
008E3C98  mov     edx, r14d
008E3C9B  lea     rdi, [r15+rax+5A10h]
008E3CA3  call    sub_10E40A0
008E3CA8  jmp     short loc_8E3CDC
008E3CAA  imul    rax, [r15+40D90h], 0F1E0h
008E3CB5  imul    rcx, [r15+4A28h], 1AE0h
008E3CC0  mov     edx, 1
008E3CC5  mov     esi, r14d
008E3CC8  add     rax, r15
008E3CCB  lea     rdi, [rcx+rax+22A20h]
008E3CD3  lea     rcx, [rbp-40h]
008E3CD7  call    sub_10EEDA0
008E3CDC  mov     rax, [r12]
008E3CE0  cmp     rax, [rbp-30h]
008E3CE4  jnz     short loc_8E3CF5
008E3CE6  add     rsp, 38h
008E3CEA  pop     rbx
008E3CEB  pop     r12
008E3CED  pop     r13
008E3CEF  pop     r14
008E3CF1  pop     r15
008E3CF3  pop     rbp
008E3CF4  retn
008E3CF5  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
