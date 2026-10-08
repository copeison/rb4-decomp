; IDA disassembly evidence for Orbis geometry and vertex shader backends.

; Range 0x8E40D0-0x8E43A5
008E40D0  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E40D1  mov     rbp, rsp
008E40D4  push    rbx
008E40D5  push    rax
008E40D6  lea     rax, unk_195F8A0
008E40DD  mov     rbx, rdi
008E40E0  add     rax, 10h
008E40E4  mov     [rbx], rax
008E40E7  call    sub_642310
008E40EC  mov     rdi, rbx
008E40EF  add     rsp, 8
008E40F3  pop     rbx
008E40F4  pop     rbp
008E40F5  jmp     nullsub_48
008E40FA  align 20h
008E4100  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4101  mov     rbp, rsp
008E4104  push    rbx
008E4105  push    rax
008E4106  lea     rax, unk_195F8A0
008E410D  mov     rbx, rdi
008E4110  add     rax, 10h
008E4114  mov     [rbx], rax
008E4117  call    sub_642310
008E411C  mov     rdi, rbx
008E411F  call    nullsub_48
008E4124  mov     rdi, rbx
008E4127  add     rsp, 8
008E412B  pop     rbx
008E412C  pop     rbp
008E412D  jmp     sub_37BF50
008E4132  align 20h
008E4140  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4141  mov     rbp, rsp
008E4144  push    r15
008E4146  push    r14
008E4148  push    r13
008E414A  push    r12
008E414C  push    rbx
008E414D  sub     rsp, 48h
008E4151  mov     rax, cs:qword_19A9B88
008E4158  lea     r15, [rbp+var_40]
008E415C  mov     r13, rdi
008E415F  mov     r14, rsi
008E4162  mov     rdi, r15
008E4165  mov     rax, [rax]
008E4168  mov     [rbp+var_30], rax
008E416C  call    sub_11B2EE0
008E4171  lea     r12, [rbp+var_58]
008E4175  mov     esi, 1
008E417A  mov     edx, 1
008E417F  mov     rdi, r12
008E4182  call    sub_37AA30
008E4187  mov     rdi, r15
008E418A  mov     rsi, r14
008E418D  call    sub_11B2F30
008E4192  mov     rdi, r12
008E4195  call    sub_37AAF0
008E419A  mov     rdx, [rbp+var_40]
008E419E  lea     rdi, [rbp+var_58]
008E41A2  lea     rsi, [rbp+var_70]
008E41A6  call    sub_10EF310
008E41AB  mov     al, cs:byte_1ADF190
008E41B1  test    al, al
008E41B3  jnz     short loc_8E41E4
008E41B5  lea     rdi, byte_1ADF190
008E41BC  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E41C1  test    eax, eax
008E41C3  jz      short loc_8E41E4
008E41C5  lea     rdi, aGpu_6; "gpu"
008E41CC  call    sub_37BA70
008E41D1  lea     rdi, byte_1ADF190
008E41D8  mov     cs:qword_1ADF188, rax
008E41DF  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E41E4  mov     rdi, cs:qword_1ADF188
008E41EB  call    sub_37A920
008E41F0  mov     edi, [rbp+var_48]
008E41F3  lea     r14, aGshader; "GShader"
008E41FA  mov     edx, 100h
008E41FF  mov     rsi, r14
008E4202  call    sub_37AE70
008E4207  mov     [r13+38h], rax
008E420B  mov     edx, 100h
008E4210  mov     rsi, r14
008E4213  mov     edi, [rbp+var_60]
008E4216  call    sub_37AE70
008E421B  mov     [r13+40h], rax
008E421F  call    sub_37A9B0
008E4224  mov     rax, [rbp+var_58]
008E4228  mov     ecx, [rax]
008E422A  shr     ecx, 18h
008E422D  movzx   esi, byte ptr [rax+rcx*4+5Ch]
008E4232  movzx   edx, byte ptr [rax+rcx*4+3Bh]
008E4237  movzx   eax, byte ptr [rax+rcx*4+5Dh]
008E423C  add     esi, edx
008E423E  mov     edx, 4
008E4243  shl     esi, 2
008E4246  lea     eax, [rsi+rax*2+2Bh]
008E424A  mov     rsi, r14
008E424D  and     eax, 0FFCh
008E4252  lea     edi, [rax+rcx*4+38h]
008E4256  call    sub_37AE70
008E425B  mov     [r13+30h], rax
008E425F  mov     rdi, [r13+38h]
008E4263  mov     rsi, [rbp+var_50]
008E4267  mov     edx, [rbp+var_48]
008E426A  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E426F  mov     rdi, [r13+40h]
008E4273  mov     rsi, [rbp+var_68]
008E4277  mov     edx, [rbp+var_60]
008E427A  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E427F  mov     rsi, [rbp+var_58]
008E4283  mov     rdi, [r13+30h]
008E4287  mov     eax, [rsi]
008E4289  shr     eax, 18h
008E428C  movzx   edx, byte ptr [rsi+rax*4+5Ch]
008E4291  movzx   ecx, byte ptr [rsi+rax*4+3Bh]
008E4296  movzx   ebx, byte ptr [rsi+rax*4+5Dh]
008E429B  add     edx, ecx
008E429D  shl     edx, 2
008E42A0  lea     ecx, [rdx+rbx*2+2Bh]
008E42A4  and     ecx, 0FFCh
008E42AA  lea     edx, [rcx+rax*4+38h]
008E42AE  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E42B3  mov     rax, [r13+30h]
008E42B7  lea     rbx, [rbp+var_40]
008E42BB  mov     rdi, rbx
008E42BE  mov     [r13+28h], rax
008E42C2  mov     rcx, [r13+38h]
008E42C6  mov     rdx, [r13+40h]
008E42CA  mov     rsi, rcx
008E42CD  shr     rcx, 28h
008E42D1  shr     rsi, 8
008E42D5  mov     [rax+8], esi
008E42D8  mov     [rax+0Ch], ecx
008E42DB  mov     rsi, rdx
008E42DE  shr     rdx, 28h
008E42E2  movzx   ecx, byte ptr [rax+3]
008E42E6  shr     rsi, 8
008E42EA  mov     [rax+rcx*4+40h], esi
008E42EE  mov     [rax+rcx*4+44h], edx
008E42F2  call    sub_11B2F00
008E42F7  mov     rdi, rbx
008E42FA  call    nullsub_240
008E42FF  mov     rax, cs:qword_19A9B88
008E4306  mov     rax, [rax]
008E4309  cmp     rax, [rbp+var_30]
008E430D  jnz     short loc_8E4320
008E430F  mov     al, 1
008E4311  add     rsp, 48h
008E4315  pop     rbx
008E4316  pop     r12
008E4318  pop     r13
008E431A  pop     r14
008E431C  pop     r15
008E431E  pop     rbp
008E431F  retn
008E4320  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E4325  align 10h
008E4330  imul    rax, [rsi+40D90h], 0E888h; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E433B  lea     rax, [rsi+rax+5728h]
008E4343  mov     rsi, [rdi+28h]
008E4347  mov     rdi, rax
008E434A  jmp     gnmx_gfx_context_set_geometry_shader
008E434F  nop
008E4350  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4351  mov     rbp, rsp
008E4354  push    r14
008E4356  push    rbx
008E4357  lea     r14, g_orbis_render_system
008E435E  mov     rbx, rdi
008E4361  mov     rsi, [rbx+30h]
008E4365  mov     rdi, [r14]
008E4368  call    orbis_defer_allocation_release
008E436D  mov     rdi, [r14]
008E4370  mov     rsi, [rbx+38h]
008E4374  call    orbis_defer_allocation_release
008E4379  mov     rdi, [r14]
008E437C  mov     rsi, [rbx+40h]
008E4380  call    orbis_defer_allocation_release
008E4385  mov     qword ptr [rbx+28h], 0
008E438D  pop     rbx
008E438E  pop     r14
008E4390  pop     rbp
008E4391  retn
008E4392  align 20h
008E43A0  mov     eax, 3; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E43A5  retn

; Range 0x8E4720-0x8E4F32
008E4720  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4721  mov     rbp, rsp
008E4724  push    rbx
008E4725  push    rax
008E4726  lea     rax, unk_195F920
008E472D  mov     rbx, rdi
008E4730  add     rax, 10h
008E4734  mov     [rbx], rax
008E4737  call    sub_642310
008E473C  mov     rdi, rbx
008E473F  add     rsp, 8
008E4743  pop     rbx
008E4744  pop     rbp
008E4745  jmp     nullsub_48
008E474A  align 10h
008E4750  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4751  mov     rbp, rsp
008E4754  push    rbx
008E4755  push    rax
008E4756  lea     rax, unk_195F920
008E475D  mov     rbx, rdi
008E4760  add     rax, 10h
008E4764  mov     [rbx], rax
008E4767  call    sub_642310
008E476C  mov     rdi, rbx
008E476F  call    nullsub_48
008E4774  mov     rdi, rbx
008E4777  add     rsp, 8
008E477B  pop     rbx
008E477C  pop     rbp
008E477D  jmp     sub_37BF50
008E4782  align 10h
008E4790  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4791  mov     rbp, rsp
008E4794  push    r15
008E4796  push    r14
008E4798  push    r13
008E479A  push    r12
008E479C  push    rbx
008E479D  sub     rsp, 38h
008E47A1  mov     rax, cs:qword_19A9B88
008E47A8  lea     r15, [rbp+var_40]
008E47AC  mov     r14, rdi
008E47AF  mov     rbx, rsi
008E47B2  mov     rdi, r15
008E47B5  mov     rax, [rax]
008E47B8  mov     [rbp+var_30], rax
008E47BC  call    sub_11B2EE0
008E47C1  lea     r12, [rbp+var_58]
008E47C5  mov     esi, 1
008E47CA  mov     edx, 1
008E47CF  mov     rdi, r12
008E47D2  call    sub_37AA30
008E47D7  mov     rdi, r15
008E47DA  mov     rsi, rbx
008E47DD  call    sub_11B2F30
008E47E2  mov     rdi, r12
008E47E5  call    sub_37AAF0
008E47EA  mov     rsi, [rbp+var_40]
008E47EE  lea     rdi, [rbp+var_58]
008E47F2  call    sub_10EF2E0
008E47F7  mov     al, cs:byte_1ADF1D0
008E47FD  test    al, al
008E47FF  jnz     short loc_8E4830
008E4801  lea     rdi, byte_1ADF1D0
008E4808  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E480D  test    eax, eax
008E480F  jz      short loc_8E4830
008E4811  lea     rdi, byte_12D1800
008E4818  call    sub_37BA70
008E481D  lea     rdi, byte_1ADF1D0
008E4824  mov     cs:qword_1ADF1C8, rax
008E482B  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E4830  mov     rdi, cs:qword_1ADF1C8
008E4837  call    sub_37A920
008E483C  mov     edi, [rbp+var_48]
008E483F  lea     rbx, aVshader; "VShader"
008E4846  mov     edx, 100h
008E484B  mov     rsi, rbx
008E484E  call    sub_37AE70
008E4853  mov     [r14+48h], rax
008E4857  call    sub_37A9B0
008E485C  mov     rax, [rbp+var_58]
008E4860  mov     rsi, rbx
008E4863  movzx   edx, byte ptr [rax+24h]
008E4867  movzx   ecx, byte ptr [rax+3]
008E486B  movzx   eax, byte ptr [rax+25h]
008E486F  add     edx, ecx
008E4871  shl     edx, 2
008E4874  lea     edi, [rdx+rax*2+2Bh]
008E4878  mov     edx, 4
008E487D  and     edi, 0FFCh
008E4883  call    sub_37AE70
008E4888  mov     [r14+38h], rax
008E488C  mov     edx, 4
008E4891  mov     rsi, rbx
008E4894  mov     rax, [rbp+var_58]
008E4898  movzx   ecx, byte ptr [rax+18h]
008E489C  movzx   eax, byte ptr [rax+3]
008E48A0  add     rax, rcx
008E48A3  lea     rdi, ds:20h[rax*4]
008E48AB  call    sub_37AE70
008E48B0  mov     [r14+40h], rax
008E48B4  mov     rdi, [r14+48h]
008E48B8  mov     rsi, [rbp+var_50]
008E48BC  mov     edx, [rbp+var_48]
008E48BF  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E48C4  mov     rsi, [rbp+var_58]
008E48C8  mov     rdi, [r14+38h]
008E48CC  movzx   ecx, byte ptr [rsi+24h]
008E48D0  movzx   eax, byte ptr [rsi+3]
008E48D4  movzx   edx, byte ptr [rsi+25h]
008E48D8  add     ecx, eax
008E48DA  shl     ecx, 2
008E48DD  lea     edx, [rcx+rdx*2+2Bh]
008E48E1  and     edx, 0FFCh
008E48E7  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E48EC  mov     rsi, [rbp+var_58]
008E48F0  mov     rdi, [r14+40h]
008E48F4  movzx   eax, byte ptr [rsi+18h]
008E48F8  movzx   ecx, byte ptr [rsi+3]
008E48FC  add     rcx, rax
008E48FF  lea     rdx, ds:20h[rcx*4]
008E4907  call    memcpy; PS4 SDK 5.008 import resolved from NID Q3VBxCXhUHs; stub: target/lib/libc_stub_weak.a
008E490C  mov     rdx, [r14+38h]
008E4910  mov     [r14+28h], rdx
008E4914  mov     rax, [r14+48h]
008E4918  mov     rsi, rax
008E491B  shr     rax, 28h
008E491F  shr     rsi, 8
008E4923  mov     [rdx+8], esi
008E4926  mov     [rdx+0Ch], eax
008E4929  mov     rcx, [r14+40h]
008E492D  mov     [r14+30h], rcx
008E4931  mov     [rcx+8], esi
008E4934  mov     [rcx+0Ch], eax
008E4937  movzx   eax, byte ptr [rdx+24h]
008E493B  test    rax, rax
008E493E  jz      loc_8E4A72
008E4944  movzx   r9d, byte ptr [rdx+3]
008E4949  cmp     al, 8
008E494B  jb      loc_8E4A3E
008E4951  mov     esi, eax
008E4953  mov     r8d, 8
008E4959  and     esi, 7
008E495C  test    al, 7
008E495E  cmovnz  r8, rsi
008E4962  mov     rsi, rax
008E4965  sub     rsi, r8
008E4968  jz      loc_8E4A3E
008E496E  vmovdqu xmm8, cs:xmmword_12D1770
008E4976  vmovdqu xmm9, cs:xmmword_12D1780
008E497E  vmovdqu xmm10, cs:xmmword_12D1790
008E4986  lea     rbx, [rdx+r9*4+40h]
008E498B  vpxor   xmm0, xmm0, xmm0
008E498F  mov     rdi, rsi
008E4992  vpxor   xmm4, xmm4, xmm4
008E4996  vpxor   xmm5, xmm5, xmm5
008E499A  vpxor   xmm6, xmm6, xmm6
008E499E  xchg    ax, ax
008E49A0  vmovq   xmm7, qword ptr [rbx-18h]
008E49A5  vmovq   xmm1, qword ptr [rbx-10h]
008E49AA  vmovq   xmm2, qword ptr [rbx-8]
008E49AF  vmovq   xmm3, qword ptr [rbx]
008E49B3  add     rbx, 20h ; ' '
008E49B7  add     rdi, 0FFFFFFFFFFFFFFF8h
008E49BB  vpshufb xmm7, xmm7, xmm8
008E49C0  vpshufb xmm1, xmm1, xmm8
008E49C5  vpshufb xmm2, xmm2, xmm8
008E49CA  vpshufb xmm3, xmm3, xmm8
008E49CF  vpxor   xmm7, xmm7, xmm9
008E49D4  vpxor   xmm1, xmm1, xmm9
008E49D9  vpxor   xmm2, xmm2, xmm9
008E49DE  vpxor   xmm3, xmm3, xmm9
008E49E3  vpcmpgtq xmm7, xmm10, xmm7
008E49E8  vpcmpgtq xmm1, xmm10, xmm1
008E49ED  vpcmpgtq xmm2, xmm10, xmm2
008E49F2  vpcmpgtq xmm3, xmm10, xmm3
008E49F7  vpsrlq  xmm7, xmm7, 3Fh ; '?'
008E49FC  vpsrlq  xmm1, xmm1, 3Fh ; '?'
008E4A01  vpsrlq  xmm2, xmm2, 3Fh ; '?'
008E4A06  vpsrlq  xmm3, xmm3, 3Fh ; '?'
008E4A0B  vpaddq  xmm0, xmm7, xmm0
008E4A0F  vpaddq  xmm4, xmm1, xmm4
008E4A13  vpaddq  xmm5, xmm2, xmm5
008E4A17  vpaddq  xmm6, xmm3, xmm6
008E4A1B  jnz     short loc_8E49A0
008E4A1D  vpaddq  xmm0, xmm4, xmm0
008E4A21  test    r8, r8
008E4A24  vpaddq  xmm0, xmm5, xmm0
008E4A28  vpaddq  xmm0, xmm6, xmm0
008E4A2C  vpshufd xmm1, xmm0, 4Eh ; 'N'
008E4A31  vpaddq  xmm0, xmm0, xmm1
008E4A35  vmovq   r13, xmm0
008E4A3A  jnz     short loc_8E4A43
008E4A3C  jmp     short loc_8E4A75
008E4A3E  xor     esi, esi
008E4A40  xor     r13d, r13d
008E4A43  mov     rdi, rax
008E4A46  add     r9, rsi
008E4A49  sub     rdi, rsi
008E4A4C  lea     rdx, [rdx+r9*4+28h]
008E4A51  nop     word ptr [rax+rax+00000000h]
008E4A60  cmp     byte ptr [rdx], 8
008E4A63  adc     r13, 0
008E4A67  add     rdx, 4
008E4A6B  dec     rdi
008E4A6E  jnz     short loc_8E4A60
008E4A70  jmp     short loc_8E4A75
008E4A72  xor     r13d, r13d
008E4A75  mov     cl, [rcx+18h]
008E4A78  cmp     al, cl
008E4A7A  jb      short loc_8E4A7E
008E4A7C  mov     ecx, eax
008E4A7E  lea     r12, [rbp+var_60]
008E4A82  mov     esi, 1
008E4A87  mov     edx, 1
008E4A8C  movzx   r15d, cl
008E4A90  mov     rdi, r12
008E4A93  call    sub_37AA30
008E4A98  lea     rdi, ds:0[r15*4]
008E4AA0  call    sub_37BF60
008E4AA5  mov     rdi, r12
008E4AA8  mov     rbx, rax
008E4AAB  call    sub_37AAF0
008E4AB0  test    r13, r13
008E4AB3  jz      short loc_8E4AC7
008E4AB5  lea     rdx, ds:0[r13*4]
008E4ABD  xor     esi, esi
008E4ABF  mov     rdi, rbx
008E4AC2  call    memset; PS4 SDK 5.008 import resolved from NID 8zTFvBIAIN8; stub: target/lib/libc_stub_weak.a
008E4AC7  mov     rcx, r15
008E4ACA  sub     rcx, r13
008E4ACD  jbe     short loc_8E4B3F
008E4ACF  cmp     rcx, 1Fh
008E4AD3  jbe     short loc_8E4B24
008E4AD5  mov     rax, rcx
008E4AD8  mov     rdx, rcx
008E4ADB  and     rax, 0FFFFFFFFFFFFFFE0h
008E4ADF  and     rdx, 0FFFFFFFFFFFFFFE0h
008E4AE3  jz      short loc_8E4B24
008E4AE5  vmovdqu ymm0, cs:ymmword_12D17E0
008E4AED  add     rax, r13
008E4AF0  lea     rsi, [rbx+r13*4+60h]
008E4AF5  mov     rdi, rdx
008E4AF8  nop     dword ptr [rax+rax+00000000h]
008E4B00  vmovdqu ymmword ptr [rsi-60h], ymm0
008E4B05  vmovdqu ymmword ptr [rsi-40h], ymm0
008E4B0A  vmovdqu ymmword ptr [rsi-20h], ymm0
008E4B0F  vmovdqu ymmword ptr [rsi], ymm0
008E4B13  sub     rsi, 0FFFFFFFFFFFFFF80h
008E4B17  add     rdi, 0FFFFFFFFFFFFFFE0h
008E4B1B  jnz     short loc_8E4B00
008E4B1D  cmp     rcx, rdx
008E4B20  jnz     short loc_8E4B30
008E4B22  jmp     short loc_8E4B3F
008E4B24  mov     rax, r13
008E4B27  nop     word ptr [rax+rax+00000000h]
008E4B30  mov     dword ptr [rbx+rax*4], 1
008E4B37  inc     rax
008E4B3A  cmp     rax, r15
008E4B3D  jb      short loc_8E4B30
008E4B3F  mov     al, cs:byte_1ADF1E0
008E4B45  test    al, al
008E4B47  jnz     short loc_8E4B78
008E4B49  lea     rdi, byte_1ADF1E0
008E4B50  call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008E4B55  test    eax, eax
008E4B57  jz      short loc_8E4B78
008E4B59  lea     rdi, byte_12D1800
008E4B60  call    sub_37BA70
008E4B65  lea     rdi, byte_1ADF1E0
008E4B6C  mov     cs:qword_1ADF1D8, rax
008E4B73  call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008E4B78  mov     rdi, cs:qword_1ADF1D8
008E4B7F  call    sub_37A920
008E4B84  mov     rdi, [r14+28h]
008E4B88  call    sub_10E9750
008E4B8D  lea     r12, aVshader; "VShader"
008E4B94  mov     edi, eax
008E4B96  mov     edx, 4
008E4B9B  mov     rsi, r12
008E4B9E  call    sub_37AE70
008E4BA3  mov     [r14+58h], rax
008E4BA7  mov     rdi, [r14+30h]
008E4BAB  call    sub_10E9F50
008E4BB0  mov     edi, eax
008E4BB2  mov     edx, 4
008E4BB7  mov     rsi, r12
008E4BBA  call    sub_37AE70
008E4BBF  mov     [r14+60h], rax
008E4BC3  call    sub_37A9B0
008E4BC8  mov     rdx, [r14+28h]
008E4BCC  mov     rdi, [r14+58h]
008E4BD0  lea     r12, [r14+50h]
008E4BD4  mov     rcx, rbx
008E4BD7  mov     r8d, r15d
008E4BDA  mov     rsi, r12
008E4BDD  call    sub_10E9990
008E4BE2  mov     rdx, [r14+30h]
008E4BE6  mov     rdi, [r14+60h]
008E4BEA  mov     rsi, r12
008E4BED  mov     rcx, rbx
008E4BF0  mov     r8d, r15d
008E4BF3  call    sub_10EA190
008E4BF8  mov     rdi, rbx
008E4BFB  call    sub_37BF70
008E4C00  lea     rbx, [rbp+var_40]
008E4C04  mov     rdi, rbx
008E4C07  call    sub_11B2F00
008E4C0C  mov     rdi, rbx
008E4C0F  call    nullsub_240
008E4C14  mov     rax, cs:qword_19A9B88
008E4C1B  mov     rax, [rax]
008E4C1E  cmp     rax, [rbp+var_30]
008E4C22  jnz     short loc_8E4C35
008E4C24  mov     al, 1
008E4C26  add     rsp, 38h
008E4C2A  pop     rbx
008E4C2B  pop     r12
008E4C2D  pop     r13
008E4C2F  pop     r14
008E4C31  pop     r15
008E4C33  pop     rbp
008E4C34  retn
008E4C35  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008E4C3A  align 20h
008E4CB0  vmovq   xmm7, qword ptr [rax-18h]
008E4CB5  vmovq   xmm1, qword ptr [rax-10h]
008E4CBA  vmovq   xmm2, qword ptr [rax-8]
008E4CBF  vmovq   xmm3, qword ptr [rax]
008E4CC3  add     rax, 20h ; ' '
008E4CC7  add     rsi, 0FFFFFFFFFFFFFFF8h
008E4CCB  vpshufb xmm7, xmm7, xmm8
008E4CD0  vpshufb xmm1, xmm1, xmm8
008E4CD5  vpshufb xmm2, xmm2, xmm8
008E4CDA  vpshufb xmm3, xmm3, xmm8
008E4CDF  vpxor   xmm7, xmm7, xmm9
008E4CE4  vpxor   xmm1, xmm1, xmm9
008E4CE9  vpxor   xmm2, xmm2, xmm9
008E4CEE  vpxor   xmm3, xmm3, xmm9
008E4CF3  vpcmpgtq xmm7, xmm10, xmm7
008E4CF8  vpcmpgtq xmm1, xmm10, xmm1
008E4CFD  vpcmpgtq xmm2, xmm10, xmm2
008E4D02  vpcmpgtq xmm3, xmm10, xmm3
008E4D07  vpsrlq  xmm7, xmm7, 3Fh ; '?'
008E4D0C  vpsrlq  xmm1, xmm1, 3Fh ; '?'
008E4D11  vpsrlq  xmm2, xmm2, 3Fh ; '?'
008E4D16  vpsrlq  xmm3, xmm3, 3Fh ; '?'
008E4D1B  vpaddq  xmm0, xmm7, xmm0
008E4D1F  vpaddq  xmm4, xmm1, xmm4
008E4D23  vpaddq  xmm5, xmm2, xmm5
008E4D27  vpaddq  xmm6, xmm3, xmm6
008E4D2B  jnz     short loc_8E4CB0
008E4D2D  vpaddq  xmm0, xmm4, xmm0
008E4D31  test    r9, r9
008E4D34  vpaddq  xmm0, xmm5, xmm0
008E4D38  vpaddq  xmm0, xmm6, xmm0
008E4D3C  vpshufd xmm1, xmm0, 4Eh ; 'N'
008E4D41  vpaddq  xmm0, xmm0, xmm1
008E4D45  vmovq   rax, xmm0
008E4D4A  jnz     short loc_8E4D52
008E4D4C  jmp     short locret_8E4D70
008E4D52  lea     rdx, [rdx+r8*4+28h]
008E4D57  nop     word ptr [rax+rax+00000000h]
008E4D60  cmp     byte ptr [rdx+rdi*4], 8
008E4D64  adc     rax, 0
008E4D68  inc     rdi
008E4D6B  cmp     rdi, rcx
008E4D6E  jb      short loc_8E4D60
008E4D70  retn
008E4D74  align 20h
008E4D80  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4D81  mov     rbp, rsp
008E4D84  push    r15
008E4D86  push    r14
008E4D88  push    r13
008E4D8A  push    r12
008E4D8C  push    rbx
008E4D8D  push    rax
008E4D8E  mov     rbx, rsi
008E4D91  mov     r12, rdi
008E4D94  mov     rax, [rbx+40D90h]
008E4D9B  test    byte ptr [rbx+4960h], 8
008E4DA2  jnz     loc_8E4E2E
008E4DA8  mov     r14, [r12+28h]
008E4DAD  mov     edx, [r12+50h]
008E4DB2  mov     r12, [r12+58h]
008E4DB7  imul    rax, 0E888h
008E4DBE  lea     r15, [rbx+rax+5A10h]
008E4DC6  lea     r13, [rbx+rax+0AD08h]
008E4DCE  test    r14, r14
008E4DD1  jz      short loc_8E4DF4
008E4DD3  cmp     [rbx+rax+13E10h], r14
008E4DDB  jz      short loc_8E4DF4
008E4DDD  mov     [rbp+var_2C], edx
008E4DE0  lea     rsi, [r14+28h]
008E4DE4  mov     rdi, r13
008E4DE7  movzx   edx, byte ptr [r14+3]
008E4DEC  call    sub_10E7BA0
008E4DF1  mov     edx, [rbp+var_2C]
008E4DF4  mov     rdi, r15
008E4DF7  mov     rsi, r14
008E4DFA  mov     rcx, r12
008E4DFD  mov     r8, r13
008E4E00  call    gnmx_gfx_context_set_vertex_shader
008E4E05  imul    rax, [rbx+40D90h], 0E888h
008E4E10  xor     esi, esi
008E4E12  xor     edx, edx
008E4E14  xor     ecx, ecx
008E4E16  xor     r8d, r8d
008E4E19  lea     rdi, [rbx+rax+5A10h]
008E4E21  lea     r9, [rbx+rax+0DD58h]
008E4E29  jmp     loc_8E4EAF
008E4E2E  mov     edx, [r12+50h]
008E4E33  imul    rax, 0E888h
008E4E3A  xor     esi, esi
008E4E3C  xor     ecx, ecx
008E4E3E  lea     rdi, [rbx+rax+5A10h]
008E4E46  lea     r8, [rbx+rax+0AD08h]
008E4E4E  call    gnmx_gfx_context_set_vertex_shader
008E4E53  imul    rax, [rbx+40D90h], 0E888h
008E4E5E  mov     r14, [r12+30h]
008E4E63  mov     ecx, [r12+50h]
008E4E68  mov     r12, [r12+60h]
008E4E6D  test    r14, r14
008E4E70  lea     r15, [rbx+rax+5A10h]
008E4E78  lea     r13, [rbx+rax+0DD58h]
008E4E80  jz      short loc_8E4EA1
008E4E82  cmp     qword ptr ds:loc_13E38[rbx+rax], r14
008E4E8A  jz      short loc_8E4EA1
008E4E8C  movzx   edx, byte ptr [r14+3]
008E4E91  lea     rsi, [r14+20h]
008E4E95  mov     rdi, r13
008E4E98  mov     ebx, ecx
008E4E9A  call    sub_10E7BA0
008E4E9F  mov     ecx, ebx
008E4EA1  xor     edx, edx
008E4EA3  mov     rdi, r15
008E4EA6  mov     rsi, r14
008E4EA9  mov     r8, r12
008E4EAC  mov     r9, r13
008E4EAF  add     rsp, 8
008E4EB3  pop     rbx
008E4EB4  pop     r12
008E4EB6  pop     r13
008E4EB8  pop     r14
008E4EBA  pop     r15
008E4EBC  pop     rbp
008E4EBD  jmp     sub_10E6C30
008E4EC2  align 10h
008E4ED0  push    rbp; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4ED1  mov     rbp, rsp
008E4ED4  push    r14
008E4ED6  push    rbx
008E4ED7  lea     r14, g_orbis_render_system
008E4EDE  mov     rbx, rdi
008E4EE1  mov     rsi, [rbx+58h]
008E4EE5  mov     rdi, [r14]
008E4EE8  call    orbis_defer_allocation_release
008E4EED  mov     rdi, [r14]
008E4EF0  mov     rsi, [rbx+60h]
008E4EF4  call    orbis_defer_allocation_release
008E4EF9  mov     rdi, [r14]
008E4EFC  mov     rsi, [rbx+38h]
008E4F00  call    orbis_defer_allocation_release
008E4F05  mov     rdi, [r14]
008E4F08  mov     rsi, [rbx+40h]
008E4F0C  call    orbis_defer_allocation_release
008E4F11  mov     rdi, [r14]
008E4F14  mov     rsi, [rbx+48h]
008E4F18  call    orbis_defer_allocation_release
008E4F1D  vxorps  xmm0, xmm0, xmm0
008E4F21  vmovups xmmword ptr [rbx+28h], xmm0
008E4F26  pop     rbx
008E4F27  pop     r14
008E4F29  pop     rbp
008E4F2A  retn
008E4F2B  align 10h
008E4F30  xor     eax, eax; Recovered Orbis shader virtual method; see docs/orbis-shader-backends.md.
008E4F32  retn
