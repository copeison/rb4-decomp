; IDA disassembly evidence for Orbis compute and pixel shader backends.

; Range 0x8E3D50-0x8E4075
008E3D50  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E3D51  mov     rbp, rsp
008E3D54  push    rbx
008E3D55  push    rax
008E3D56  lea     rax, unk_195F860
008E3D5D  mov     rbx, rdi
008E3D60  add     rax, 10h
008E3D64  mov     [rbx], rax
008E3D67  call    sub_642310
008E3D6C  mov     rdi, rbx
008E3D6F  add     rsp, 8
008E3D73  pop     rbx
008E3D74  pop     rbp
008E3D75  jmp     nullsub_48
008E3D7A  align 20h
008E3D80  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E3D81  mov     rbp, rsp
008E3D84  push    rbx
008E3D85  push    rax
008E3D86  lea     rax, unk_195F860
008E3D8D  mov     rbx, rdi
008E3D90  add     rax, 10h
008E3D94  mov     [rbx], rax
008E3D97  call    sub_642310
008E3D9C  mov     rdi, rbx
008E3D9F  call    nullsub_48
008E3DA4  mov     rdi, rbx
008E3DA7  add     rsp, 8
008E3DAB  pop     rbx
008E3DAC  pop     rbp
008E3DAD  jmp     sub_37BF50
008E3DB2  align 20h
008E3DC0  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E3DC1  mov     rbp, rsp
008E3DC4  push    r15
008E3DC6  push    r14
008E3DC8  push    r13
008E3DCA  push    r12
008E3DCC  push    rbx
008E3DCD  sub     rsp, 38h
008E3DD1  mov     r13, cs:qword_19A9B88
008E3DD8  lea     r15, [rbp+var_40]
008E3DDC  mov     rbx, rdi
008E3DDF  mov     r14, rsi
008E3DE2  mov     rdi, r15
008E3DE5  mov     rax, [r13+0]
008E3DE9  mov     [rbp+var_30], rax
008E3DED  call    sub_11B2EE0
008E3DF2  lea     r12, [rbp+var_58]
008E3DF6  mov     esi, 1
008E3DFB  mov     edx, 1
008E3E00  mov     rdi, r12
008E3E03  call    sub_37AA30
008E3E08  mov     rdi, r15
008E3E0B  mov     rsi, r14
008E3E0E  call    sub_11B2F30
008E3E13  mov     rdi, r12
008E3E16  call    sub_37AAF0
008E3E1B  mov     rsi, [rbp+var_40]
008E3E1F  lea     rdi, [rbp+var_58]
008E3E23  call    sub_10EF2E0
008E3E28  mov     al, cs:byte_1ADF170
008E3E2E  test    al, al
008E3E30  jnz     short loc_8E3E61
008E3E32  lea     rdi, byte_1ADF170
008E3E39  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E3E3E  test    eax, eax
008E3E40  jz      short loc_8E3E61
008E3E42  lea     rdi, aGpu_5; "gpu"
008E3E49  call    sub_37BA70
008E3E4E  lea     rdi, byte_1ADF170
008E3E55  mov     cs:qword_1ADF168, rax
008E3E5C  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E3E61  mov     rdi, cs:qword_1ADF168
008E3E68  call    sub_37A920
008E3E6D  mov     edi, [rbp+var_48]
008E3E70  lea     r14, aCshader; "CShader"
008E3E77  mov     edx, 100h
008E3E7C  mov     rsi, r14
008E3E7F  call    sub_37AE70
008E3E84  mov     [rbx+38h], rax
008E3E88  call    sub_37A9B0
008E3E8D  mov     rax, [rbp+var_58]
008E3E91  mov     edx, 4
008E3E96  mov     rsi, r14
008E3E99  mov     ecx, [rax]
008E3E9B  movzx   eax, word ptr [rax+24h]
008E3E9F  shr     ecx, 18h
008E3EA2  and     eax, 0FCh
008E3EA7  lea     edi, [rax+rcx*4+28h]
008E3EAB  call    sub_37AE70
008E3EB0  mov     [rbx+30h], rax
008E3EB4  mov     rdi, [rbx+38h]
008E3EB8  mov     rsi, [rbp+var_50]
008E3EBC  mov     edx, [rbp+var_48]
008E3EBF  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E3EC4  mov     rsi, [rbp+var_58]
008E3EC8  mov     rdi, [rbx+30h]
008E3ECC  mov     eax, [rsi]
008E3ECE  movzx   ecx, word ptr [rsi+24h]
008E3ED2  shr     eax, 18h
008E3ED5  and     ecx, 0FCh
008E3EDB  lea     edx, [rcx+rax*4+28h]
008E3EDF  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E3EE4  mov     rax, [rbx+30h]
008E3EE8  mov     [rbx+28h], rax
008E3EEC  mov     rcx, [rbx+38h]
008E3EF0  lea     rbx, [rbp+var_40]
008E3EF4  mov     rdi, rbx
008E3EF7  mov     rdx, rcx
008E3EFA  shr     rcx, 28h
008E3EFE  shr     rdx, 8
008E3F02  mov     [rax+8], edx
008E3F05  mov     [rax+0Ch], ecx
008E3F08  call    sub_11B2F00
008E3F0D  mov     rdi, rbx
008E3F10  call    nullsub_240
008E3F15  mov     rax, [r13+0]
008E3F19  cmp     rax, [rbp+var_30]
008E3F1D  jnz     short loc_8E3F30
008E3F1F  mov     al, 1
008E3F21  add     rsp, 38h
008E3F25  pop     rbx
008E3F26  pop     r12
008E3F28  pop     r13
008E3F2A  pop     r14
008E3F2C  pop     r15
008E3F2E  pop     rbp
008E3F2F  retn
008E3F30  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E3F35  align 20h
008E3F40  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E3F41  mov     rbp, rsp
008E3F44  push    r15
008E3F46  push    r14
008E3F48  push    r13
008E3F4A  push    r12
008E3F4C  push    rbx
008E3F4D  push    rax
008E3F4E  mov     rbx, rsi
008E3F51  mov     eax, [rbx+4A24h]
008E3F57  cmp     eax, 1
008E3F5A  jz      short loc_8E3FBF
008E3F5C  test    eax, eax
008E3F5E  jnz     loc_8E401A
008E3F64  imul    rax, [rbx+40D90h], 0E888h
008E3F6F  mov     r14, [rdi+28h]
008E3F73  test    r14, r14
008E3F76  lea     r12, [rbx+rax+5A10h]
008E3F7E  lea     r15, [rbx+rax+7CB8h]
008E3F86  jz      short loc_8E3FA3
008E3F88  cmp     [rbx+rax+13E20h], r14
008E3F90  jz      short loc_8E3FA3
008E3F92  movzx   edx, byte ptr [r14+3]
008E3F97  lea     rsi, [r14+28h]
008E3F9B  mov     rdi, r15
008E3F9E  call    sub_10E7BA0
008E3FA3  mov     rdi, r12
008E3FA6  mov     rsi, r14
008E3FA9  mov     rdx, r15
008E3FAC  add     rsp, 8
008E3FB0  pop     rbx
008E3FB1  pop     r12
008E3FB3  pop     r13
008E3FB5  pop     r14
008E3FB7  pop     r15
008E3FB9  pop     rbp
008E3FBA  jmp     gnmx_gfx_context_set_compute_shader
008E3FBF  imul    r12, [rbx+40D90h], 0F1E0h
008E3FCA  imul    r13, [rbx+4A28h], 1AE0h
008E3FD5  mov     r14, [rdi+28h]
008E3FD9  xor     esi, esi
008E3FDB  mov     rdx, r14
008E3FDE  add     r12, rbx
008E3FE1  lea     r15, sub_24330[r13+r12]
008E3FE9  mov     rdi, r15
008E3FEC  call    sub_10EDD90
008E3FF1  lea     rdi, [r13+r12+22A20h]
008E3FF9  mov     rsi, r14
008E3FFC  mov     rdx, r15
008E3FFF  call    sub_10EED20
008E4004  mov     rdi, rbx
008E4007  add     rsp, 8
008E400B  pop     rbx
008E400C  pop     r12
008E400E  pop     r13
008E4010  pop     r14
008E4012  pop     r15
008E4014  pop     rbp
008E4015  jmp     loc_6BD930
008E401A  add     rsp, 8
008E401E  pop     rbx
008E401F  pop     r12
008E4021  pop     r13
008E4023  pop     r14
008E4025  pop     r15
008E4027  pop     rbp
008E4028  retn
008E4029  nop
008E402A  nop
008E402B  nop
008E402C  nop
008E402D  nop
008E402E  nop
008E402F  nop
008E4030  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4031  mov     rbp, rsp
008E4034  push    r14
008E4036  push    rbx
008E4037  lea     r14, g_orbis_render_system
008E403E  mov     rbx, rdi
008E4041  mov     rsi, [rbx+30h]
008E4045  mov     rdi, [r14]
008E4048  call    orbis_defer_allocation_release
008E404D  mov     rdi, [r14]
008E4050  mov     rsi, [rbx+38h]
008E4054  call    orbis_defer_allocation_release
008E4059  mov     qword ptr [rbx+28h], 0
008E4061  pop     rbx
008E4062  pop     r14
008E4064  pop     rbp
008E4065  retn
008E4066  align 10h
008E4070  mov     eax, 5; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4075  retn

; Range 0x8E4410-0x8E46B5
008E4410  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4411  mov     rbp, rsp
008E4414  push    rbx
008E4415  push    rax
008E4416  lea     rax, unk_195F8E0
008E441D  mov     rbx, rdi
008E4420  add     rax, 10h
008E4424  mov     [rbx], rax
008E4427  call    sub_642310
008E442C  mov     rdi, rbx
008E442F  add     rsp, 8
008E4433  pop     rbx
008E4434  pop     rbp
008E4435  jmp     nullsub_48
008E443A  align 20h
008E4440  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4441  mov     rbp, rsp
008E4444  push    rbx
008E4445  push    rax
008E4446  lea     rax, unk_195F8E0
008E444D  mov     rbx, rdi
008E4450  add     rax, 10h
008E4454  mov     [rbx], rax
008E4457  call    sub_642310
008E445C  mov     rdi, rbx
008E445F  call    nullsub_48
008E4464  mov     rdi, rbx
008E4467  add     rsp, 8
008E446B  pop     rbx
008E446C  pop     rbp
008E446D  jmp     sub_37BF50
008E4472  align 20h
008E4480  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4481  mov     rbp, rsp
008E4484  push    r15
008E4486  push    r14
008E4488  push    r13
008E448A  push    r12
008E448C  push    rbx
008E448D  sub     rsp, 38h
008E4491  mov     r13, cs:qword_19A9B88
008E4498  lea     r15, [rbp+var_40]
008E449C  mov     rbx, rdi
008E449F  mov     r14, rsi
008E44A2  mov     rdi, r15
008E44A5  mov     rax, [r13+0]
008E44A9  mov     [rbp+var_30], rax
008E44AD  call    sub_11B2EE0
008E44B2  lea     r12, [rbp+var_58]
008E44B6  mov     esi, 1
008E44BB  mov     edx, 1
008E44C0  mov     rdi, r12
008E44C3  call    sub_37AA30
008E44C8  mov     rdi, r15
008E44CB  mov     rsi, r14
008E44CE  call    sub_11B2F30
008E44D3  mov     rdi, r12
008E44D6  call    sub_37AAF0
008E44DB  mov     rsi, [rbp+var_40]
008E44DF  lea     rdi, [rbp+var_58]
008E44E3  call    sub_10EF2E0
008E44E8  mov     al, cs:byte_1ADF1B0
008E44EE  test    al, al
008E44F0  jnz     short loc_8E4521
008E44F2  lea     rdi, byte_1ADF1B0
008E44F9  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E44FE  test    eax, eax
008E4500  jz      short loc_8E4521
008E4502  lea     rdi, aGpu_7; "gpu"
008E4509  call    sub_37BA70
008E450E  lea     rdi, byte_1ADF1B0
008E4515  mov     cs:qword_1ADF1A8, rax
008E451C  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E4521  mov     rdi, cs:qword_1ADF1A8
008E4528  call    sub_37A920
008E452D  mov     edi, [rbp+var_48]
008E4530  lea     r14, aPshader; "PShader"
008E4537  mov     edx, 100h
008E453C  mov     rsi, r14
008E453F  call    sub_37AE70
008E4544  mov     [rbx+38h], rax
008E4548  call    sub_37A9B0
008E454D  mov     rax, [rbp+var_58]
008E4551  mov     edx, 4
008E4556  mov     rsi, r14
008E4559  mov     ecx, [rax]
008E455B  movzx   eax, byte ptr [rax+38h]
008E455F  shr     ecx, 18h
008E4562  add     eax, eax
008E4564  lea     edi, [rax+rcx*4+3Fh]
008E4568  and     edi, 0FFCh
008E456E  call    sub_37AE70
008E4573  mov     [rbx+30h], rax
008E4577  mov     rdi, [rbx+38h]
008E457B  mov     rsi, [rbp+var_50]
008E457F  mov     edx, [rbp+var_48]
008E4582  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E4587  mov     rsi, [rbp+var_58]
008E458B  mov     rdi, [rbx+30h]
008E458F  mov     eax, [rsi]
008E4591  movzx   ecx, byte ptr [rsi+38h]
008E4595  shr     eax, 18h
008E4598  add     ecx, ecx
008E459A  lea     edx, [rcx+rax*4+3Fh]
008E459E  and     edx, 0FFCh
008E45A4  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E45A9  mov     rax, [rbx+30h]
008E45AD  mov     [rbx+28h], rax
008E45B1  mov     rcx, [rbx+38h]
008E45B5  lea     rbx, [rbp+var_40]
008E45B9  mov     rdi, rbx
008E45BC  mov     rdx, rcx
008E45BF  shr     rcx, 28h
008E45C3  shr     rdx, 8
008E45C7  mov     [rax+8], edx
008E45CA  mov     [rax+0Ch], ecx
008E45CD  call    sub_11B2F00
008E45D2  mov     rdi, rbx
008E45D5  call    nullsub_240
008E45DA  mov     rax, [r13+0]
008E45DE  cmp     rax, [rbp+var_30]
008E45E2  jnz     short loc_8E45F5
008E45E4  mov     al, 1
008E45E6  add     rsp, 38h
008E45EA  pop     rbx
008E45EB  pop     r12
008E45ED  pop     r13
008E45EF  pop     r14
008E45F1  pop     r15
008E45F3  pop     rbp
008E45F4  retn
008E45F5  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E45FA  align 20h
008E4600  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4601  mov     rbp, rsp
008E4604  push    r15
008E4606  push    r14
008E4608  push    r12
008E460A  push    rbx
008E460B  mov     rbx, rsi
008E460E  mov     r14, [rdi+28h]
008E4612  imul    rax, [rbx+40D90h], 0E888h
008E461D  test    r14, r14
008E4620  lea     r12, [rbx+rax+5A10h]
008E4628  lea     r15, [rbx+rax+94E0h]
008E4630  jz      short loc_8E464D
008E4632  cmp     [rbx+rax+13E18h], r14
008E463A  jz      short loc_8E464D
008E463C  movzx   edx, byte ptr [r14+3]
008E4641  lea     rsi, [r14+3Ch]
008E4645  mov     rdi, r15
008E4648  call    sub_10E7BA0
008E464D  mov     rdi, r12
008E4650  mov     rsi, r14
008E4653  mov     rdx, r15
008E4656  call    gnmx_gfx_context_set_pixel_shader
008E465B  mov     esi, 1
008E4660  mov     rdi, rbx
008E4663  pop     rbx
008E4664  pop     r12
008E4666  pop     r14
008E4668  pop     r15
008E466A  pop     rbp
008E466B  jmp     loc_8EA7D0
008E4670  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4671  mov     rbp, rsp
008E4674  push    r14
008E4676  push    rbx
008E4677  lea     r14, g_orbis_render_system
008E467E  mov     rbx, rdi
008E4681  mov     rsi, [rbx+30h]
008E4685  mov     rdi, [r14]
008E4688  call    orbis_defer_allocation_release
008E468D  mov     rdi, [r14]
008E4690  mov     rsi, [rbx+38h]
008E4694  call    orbis_defer_allocation_release
008E4699  mov     qword ptr [rbx+28h], 0
008E46A1  pop     rbx
008E46A2  pop     r14
008E46A4  pop     rbp
008E46A5  retn
008E46A6  align 10h
008E46B0  mov     eax, 4; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E46B5  retn
