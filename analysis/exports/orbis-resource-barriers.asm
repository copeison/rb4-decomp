00000000008EAA10  push    rbp; Processes 32-byte transition/aliasing/UAV barriers, including split barriers, metadata resolves, cache actions, and cross-queue synchronization.
00000000008EAA11  mov     rbp, rsp
00000000008EAA14  push    r15
00000000008EAA16  push    r14
00000000008EAA18  push    r13
00000000008EAA1A  push    r12
00000000008EAA1C  push    rbx
00000000008EAA1D  sub     rsp, 0A8h
00000000008EAA24  mov     r15, cs:qword_19A9B88
00000000008EAA2B  mov     rbx, rsi
00000000008EAA2E  mov     r12, rdi
00000000008EAA31  test    rbx, rbx
00000000008EAA34  mov     rax, [r15]
00000000008EAA37  mov     [rbp-30h], rax
00000000008EAA3B  mov     qword ptr [rbp-78h], 0
00000000008EAA43  jz      loc_8EB3B7
00000000008EAA49  xor     eax, eax
00000000008EAA4B  mov     r14, rdx
00000000008EAA4E  xor     r13d, r13d
00000000008EAA51  mov     [rbp-0B8h], rdx
00000000008EAA58  mov     dword ptr [rbp-8Ch], 0
00000000008EAA62  mov     [rbp-0A8h], rax
00000000008EAA69  jmp     loc_8EAF90
00000000008EAA6E  mov     rsi, [r14+8]
00000000008EAA72  mov     rdi, r12
00000000008EAA75  lea     rdx, [rbp-78h]
00000000008EAA79  mov     r15d, r8d
00000000008EAA7C  call    orbis_render_context_signal_resource
00000000008EAA81  or      [rbp-8Ch], r15d
00000000008EAA88  jmp     loc_8EB1F0
00000000008EAA8D  mov     [rbp-0A0h], r8d
00000000008EAA94  mov     [rbp-0B0h], rcx
00000000008EAA9B  mov     rdi, [r14+8]
00000000008EAA9F  mov     rax, [rdi]
00000000008EAAA2  call    qword ptr [rax+10h]
00000000008EAAA5  cmp     eax, 5
00000000008EAAA8  jz      loc_8EAB7E
00000000008EAAAE  cmp     eax, 1
00000000008EAAB1  jnz     loc_8EAC02
00000000008EAAB7  mov     rdi, [r14+8]
00000000008EAABB  call    sub_8D71E0
00000000008EAAC0  vmovups ymm0, ymmword ptr [rax]
00000000008EAAC4  vmovups ymm1, ymmword ptr [rax+20h]
00000000008EAAC9  vmovups ymmword ptr [rbp-50h], ymm1
00000000008EAACE  vmovups ymmword ptr [rbp-70h], ymm0
00000000008EAAD3  jmp     loc_8EAD7A
00000000008EAAD8  mov     rdi, [r14+8]
00000000008EAADC  mov     rsi, 0FFFFFFFFFFFFFFFFh
00000000008EAAE3  mov     [rbp-88h], rdi
00000000008EAAEA  call    sub_8E65C0
00000000008EAAEF  vmovups ymm0, ymmword ptr [rax]
00000000008EAAF3  vmovups ymm1, ymmword ptr [rax+14h]
00000000008EAAF8  vmovups ymmword ptr [rbp-5Ch], ymm1
00000000008EAAFD  vmovups ymmword ptr [rbp-70h], ymm0
00000000008EAB02  mov     rax, [r14+10h]
00000000008EAB06  cmp     rax, 0FFFFFFFFFFFFFFFFh
00000000008EAB0A  jz      loc_8EAC24
00000000008EAB10  mov     rdi, [rbp-88h]
00000000008EAB17  mov     [rbp-98h], rax
00000000008EAB1E  mov     rax, [rdi]
00000000008EAB21  call    qword ptr [rax+50h]
00000000008EAB24  mov     rcx, rax
00000000008EAB27  mov     rax, [rbp-98h]
00000000008EAB2E  xor     edx, edx
00000000008EAB30  inc     rcx
00000000008EAB33  div     rcx
00000000008EAB36  mov     ecx, [rbp-50h]
00000000008EAB39  mov     edx, 0FF001800h
00000000008EAB3E  and     ecx, edx
00000000008EAB40  mov     edx, eax
00000000008EAB42  shl     eax, 0Dh
00000000008EAB45  and     edx, 7FFh
00000000008EAB4B  and     eax, 0FFE000h
00000000008EAB50  or      edx, ecx
00000000008EAB52  or      edx, eax
00000000008EAB54  mov     [rbp-50h], edx
00000000008EAB57  jmp     loc_8EAC4D
00000000008EAB5C  mov     rdi, [r14+8]
00000000008EAB60  mov     rax, [rdi]
00000000008EAB63  call    qword ptr [rax+10h]
00000000008EAB66  cmp     eax, 1
00000000008EAB69  jz      loc_8EAC4D
00000000008EAB6F  mov     rdi, [r14+8]
00000000008EAB73  mov     rax, [rdi]
00000000008EAB76  call    qword ptr [rax+10h]
00000000008EAB79  jmp     loc_8EAC4D
00000000008EAB7E  mov     rdi, [r14+8]
00000000008EAB82  mov     rsi, 0FFFFFFFFFFFFFFFFh
00000000008EAB89  mov     [rbp-88h], rdi
00000000008EAB90  call    sub_8E6590
00000000008EAB95  vmovups ymm0, ymmword ptr [rax]
00000000008EAB99  vmovups ymm1, ymmword ptr [rax+20h]
00000000008EAB9E  vmovups ymmword ptr [rbp-50h], ymm1
00000000008EABA3  vmovups ymmword ptr [rbp-70h], ymm0
00000000008EABA8  mov     rax, [r14+10h]
00000000008EABAC  cmp     rax, 0FFFFFFFFFFFFFFFFh
00000000008EABB0  jz      loc_8EAD51
00000000008EABB6  mov     rdi, [rbp-88h]
00000000008EABBD  mov     [rbp-98h], rax
00000000008EABC4  mov     rax, [rdi]
00000000008EABC7  call    qword ptr [rax+50h]
00000000008EABCA  mov     rcx, rax
00000000008EABCD  mov     rax, [rbp-98h]
00000000008EABD4  xor     edx, edx
00000000008EABD6  inc     rcx
00000000008EABD9  div     rcx
00000000008EABDC  mov     ecx, [rbp-64h]
00000000008EABDF  mov     edx, 0FF001800h
00000000008EABE4  and     ecx, edx
00000000008EABE6  mov     edx, eax
00000000008EABE8  shl     eax, 0Dh
00000000008EABEB  and     edx, 7FFh
00000000008EABF1  and     eax, 0FFE000h
00000000008EABF6  or      edx, ecx
00000000008EABF8  or      edx, eax
00000000008EABFA  mov     [rbp-64h], edx
00000000008EABFD  jmp     loc_8EAD7A
00000000008EAC02  mov     rdi, [r14+8]
00000000008EAC06  mov     rax, [rdi]
00000000008EAC09  call    qword ptr [rax+10h]
00000000008EAC0C  cmp     eax, 1
00000000008EAC0F  jz      loc_8EAD7A
00000000008EAC15  mov     rdi, [r14+8]
00000000008EAC19  mov     rax, [rdi]
00000000008EAC1C  call    qword ptr [rax+10h]
00000000008EAC1F  jmp     loc_8EAD7A
00000000008EAC24  mov     rax, [rbp-88h]
00000000008EAC2B  mov     ecx, [rbp-50h]
00000000008EAC2E  mov     edx, 0FF001800h
00000000008EAC33  mov     eax, [rax+80h]
00000000008EAC39  and     ecx, edx
00000000008EAC3B  shl     eax, 0Dh
00000000008EAC3E  add     eax, 0FFE000h
00000000008EAC43  and     eax, 0FFE000h
00000000008EAC48  or      eax, ecx
00000000008EAC4A  mov     [rbp-50h], eax
00000000008EAC4D  imul    rax, [r12+40D90h], 0E888h
00000000008EAC59  mov     rdi, r15
00000000008EAC5C  mov     [rbp-98h], rax
00000000008EAC63  call    sub_10C4C70
00000000008EAC68  mov     [rbp-88h], eax
00000000008EAC6E  mov     eax, [rbp-50h]
00000000008EAC71  mov     edi, [rbp-70h]
00000000008EAC74  mov     ecx, 0B0Dh
00000000008EAC79  mov     r15d, 1
00000000008EAC7F  bextr   ecx, eax, ecx
00000000008EAC84  and     eax, 7FFh
00000000008EAC89  and     edi, 3
00000000008EAC8C  sub     r15d, eax
00000000008EAC8F  add     r15d, ecx
00000000008EAC92  call    sub_10C2C20
00000000008EAC97  lea     rdi, [rbp-80h]
00000000008EAC9B  mov     [rbp-80h], eax
00000000008EAC9E  call    gnm_data_format_bytes_per_element
00000000008EACA3  mov     edx, [rbp-54h]
00000000008EACA6  movzx   ecx, byte ptr [rbp-70h]
00000000008EACAA  mov     esi, [rbp-88h]
00000000008EACB0  mov     r8d, [rbp-0A0h]
00000000008EACB7  mov     r9d, 4000000h
00000000008EACBD  mov     dword ptr [rsp], 1
00000000008EACC4  shl     edx, 6
00000000008EACC7  shr     cl, 2
00000000008EACCA  and     edx, 0FFFFFC0h
00000000008EACD0  and     cl, 3
00000000008EACD3  add     edx, 40h ; '@'
00000000008EACD6  imul    edx, eax
00000000008EACD9  mov     rax, [rbp-98h]
00000000008EACE0  shl     edx, cl
00000000008EACE2  mov     ecx, 4000h
00000000008EACE7  imul    edx, r15d
00000000008EACEB  lea     r15, [rbp-70h]
00000000008EACEF  lea     rdi, [r12+rax+5728h]
00000000008EACF7  shr     edx, 8
00000000008EACFA  call    gnm_draw_command_buffer_wait_for_graphics_writes
00000000008EACFF  mov     rdi, r15
00000000008EAD02  call    sub_10C46C0
00000000008EAD07  cmp     eax, 1
00000000008EAD0A  jnz     short loc_8EAD30
00000000008EAD0C  test    byte ptr [rbp-46h], 2
00000000008EAD10  jz      short loc_8EAD30
00000000008EAD12  imul    rax, [r12+40D90h], 0E888h
00000000008EAD1E  mov     esi, 2Ch ; ','
00000000008EAD23  lea     rdi, [r12+rax+5728h]
00000000008EAD2B  call    gnm_draw_command_buffer_emit_event
00000000008EAD30  imul    rax, [r12+40D90h], 0E888h
00000000008EAD3C  mov     rsi, r15
00000000008EAD3F  lea     rdi, [r12+rax+5728h]
00000000008EAD47  call    gnmx_decompress_depth_surface
00000000008EAD4C  jmp     loc_8EAE8F
00000000008EAD51  mov     rax, [rbp-88h]
00000000008EAD58  mov     ecx, [rbp-64h]
00000000008EAD5B  mov     edx, 0FF001800h
00000000008EAD60  mov     eax, [rax+80h]
00000000008EAD66  and     ecx, edx
00000000008EAD68  shl     eax, 0Dh
00000000008EAD6B  add     eax, 0FFE000h
00000000008EAD70  and     eax, 0FFE000h
00000000008EAD75  or      eax, ecx
00000000008EAD77  mov     [rbp-64h], eax
00000000008EAD7A  imul    rax, [r12+40D90h], 0E888h
00000000008EAD86  mov     rdi, r15
00000000008EAD89  mov     [rbp-98h], rax
00000000008EAD90  call    sub_10DB1F0
00000000008EAD95  mov     [rbp-88h], eax
00000000008EAD9B  mov     eax, [rbp-64h]
00000000008EAD9E  mov     ecx, 0B0Dh
00000000008EADA3  mov     rdi, r15
00000000008EADA6  mov     r15d, 1
00000000008EADAC  bextr   ecx, eax, ecx
00000000008EADB1  and     eax, 7FFh
00000000008EADB6  sub     r15d, eax
00000000008EADB9  add     r15d, ecx
00000000008EADBC  call    sub_10DAF60
00000000008EADC1  lea     rdi, [rbp-80h]
00000000008EADC5  mov     [rbp-80h], eax
00000000008EADC8  call    gnm_data_format_bytes_per_element
00000000008EADCD  mov     edx, [rbp-68h]
00000000008EADD0  mov     ecx, [rbp-5Ch]
00000000008EADD3  mov     esi, [rbp-88h]
00000000008EADD9  mov     r8d, [rbp-0A0h]
00000000008EADE0  mov     r9d, 2000000h
00000000008EADE6  mov     dword ptr [rsp], 1
00000000008EADED  shl     edx, 6
00000000008EADF0  shr     ecx, 0Fh
00000000008EADF3  and     edx, 0FFFFFC0h
00000000008EADF9  and     cl, 3
00000000008EADFC  add     edx, 40h ; '@'
00000000008EADFF  imul    edx, eax
00000000008EAE02  mov     rax, [rbp-98h]
00000000008EAE09  shl     edx, cl
00000000008EAE0B  mov     ecx, 3FC0h
00000000008EAE10  imul    edx, r15d
00000000008EAE14  lea     rdi, [r12+rax+5728h]
00000000008EAE1C  shr     edx, 8
00000000008EAE1F  call    gnm_draw_command_buffer_wait_for_graphics_writes
00000000008EAE24  imul    rax, [r12+40D90h], 0E888h
00000000008EAE30  mov     esi, 31h ; '1'
00000000008EAE35  lea     rdi, [r12+rax+5728h]
00000000008EAE3D  call    gnm_draw_command_buffer_emit_event
00000000008EAE42  imul    rax, [r12+40D90h], 0E888h
00000000008EAE4E  test    byte ptr [rbp-5Dh], 10h
00000000008EAE52  lea     rdi, [r12+rax+5728h]
00000000008EAE5A  jnz     short loc_8EAE86
00000000008EAE5C  lea     r15, [rbp-70h]
00000000008EAE60  mov     rsi, r15
00000000008EAE63  call    gnmx_eliminate_fast_clear
00000000008EAE68  imul    rax, [r12+40D90h], 0E888h
00000000008EAE74  mov     rsi, r15
00000000008EAE77  lea     rdi, [r12+rax+5728h]
00000000008EAE7F  call    gnmx_decompress_fmask_surface
00000000008EAE84  jmp     short loc_8EAE8F
00000000008EAE86  lea     rsi, [rbp-70h]
00000000008EAE8A  call    gnmx_decompress_dcc_surface
00000000008EAE8F  mov     qword ptr ds:sub_44880[r12], 0FFFFFFFFFFFFFFFFh
00000000008EAE9B  imul    rax, [r12+40D90h], 0E888h
00000000008EAEA7  lea     rdi, [r12+rax+5A10h]
00000000008EAEAF  call    gnmx_gfx_context_reset_command_state
00000000008EAEB4  mov     byte ptr ds:loc_44888[r12], 0
00000000008EAEBD  mov     esi, 1
00000000008EAEC2  mov     edx, 0CCh
00000000008EAEC7  imul    rax, [r12+40D90h], 0E888h
00000000008EAED3  lea     rdi, [r12+rax+5728h]
00000000008EAEDB  call    sub_10C5F80
00000000008EAEE0  mov     byte ptr [r12+44889h], 1
00000000008EAEE9  mov     qword ptr [rbp-80h], 0
00000000008EAEF1  mov     r15d, [r12+4A24h]
00000000008EAEF9  test    r15d, r15d
00000000008EAEFC  jz      short loc_8EAF61
00000000008EAEFE  mov     rax, [r12+4A28h]
00000000008EAF06  xor     esi, esi
00000000008EAF08  xor     edx, edx
00000000008EAF0A  mov     rdi, r12
00000000008EAF0D  mov     [rbp-0A0h], rax
00000000008EAF14  call    sub_6BD8A0
00000000008EAF19  mov     rsi, [r14+8]
00000000008EAF1D  mov     rdi, r12
00000000008EAF20  lea     rdx, [rbp-80h]
00000000008EAF24  call    orbis_render_context_signal_resource
00000000008EAF29  mov     eax, [r14+4]
00000000008EAF2D  mov     rdx, [rbp-0A0h]
00000000008EAF34  mov     rdi, r12
00000000008EAF37  mov     esi, r15d
00000000008EAF3A  mov     [rbp-88h], eax
00000000008EAF40  call    sub_6BD8A0
00000000008EAF45  mov     rax, [rbp-0B0h]
00000000008EAF4C  cmp     dword ptr [rbp-88h], 1
00000000008EAF53  jz      loc_8EB1F0
00000000008EAF59  mov     rsi, [rax]
00000000008EAF5C  jmp     loc_8EB1DA
00000000008EAF61  cmp     dword ptr [r14+4], 1
00000000008EAF66  mov     al, 1
00000000008EAF68  jnz     short loc_8EAF81
00000000008EAF6A  mov     rsi, [r14+8]
00000000008EAF6E  mov     rdi, r12
00000000008EAF71  lea     rdx, [rbp-80h]
00000000008EAF75  call    orbis_render_context_signal_resource
00000000008EAF7A  mov     rax, [rbp-0A8h]
00000000008EAF81  mov     [rbp-0A8h], rax
00000000008EAF88  jmp     loc_8EB1F0
00000000008EAF8D  align 10h
00000000008EAF90  mov     eax, [r14]
00000000008EAF93  cmp     eax, 2
00000000008EAF96  jz      short loc_8EAFE0
00000000008EAF98  test    eax, eax
00000000008EAF9A  jnz     loc_8EB1F0
00000000008EAFA0  mov     eax, [r14+1Ch]
00000000008EAFA4  cmp     [r14+18h], eax
00000000008EAFA8  jz      loc_8EB1F0
00000000008EAFAE  mov     r15, r13
00000000008EAFB1  xor     r8d, r8d
00000000008EAFB4  shl     r15, 5
00000000008EAFB8  cmp     eax, 0FFh
00000000008EAFBD  jg      short loc_8EB00F
00000000008EAFBF  cmp     eax, 4
00000000008EAFC2  jz      loc_8EB129
00000000008EAFC8  cmp     eax, 8
00000000008EAFCB  jz      short loc_8EB024
00000000008EAFCD  cmp     eax, 10h
00000000008EAFD0  jz      short loc_8EB02A
00000000008EAFD2  jmp     short loc_8EB024
00000000008EAFD4  align 20h
00000000008EAFE0  mov     eax, [r14+4]
00000000008EAFE4  cmp     eax, 2
00000000008EAFE7  jz      loc_8EB1D6
00000000008EAFED  cmp     eax, 1
00000000008EAFF0  jz      short loc_8EB060
00000000008EAFF2  test    eax, eax
00000000008EAFF4  jnz     loc_8EB1F0
00000000008EAFFA  or      dword ptr [rbp-8Ch], 10h
00000000008EB001  mov     al, 1
00000000008EB003  mov     [rbp-0A8h], rax
00000000008EB00A  jmp     loc_8EB1F0
00000000008EB00F  cmp     eax, 100h
00000000008EB014  jz      short loc_8EB024
00000000008EB016  cmp     eax, 400h
00000000008EB01B  jz      short loc_8EB024
00000000008EB01D  cmp     eax, 1000h
00000000008EB022  jz      short loc_8EB02A
00000000008EB024  mov     r8d, 38h ; '8'
00000000008EB02A  mov     rax, [rbp-0B8h]
00000000008EB031  lea     rcx, [rax+r15+8]
00000000008EB036  mov     eax, [r14+18h]
00000000008EB03A  cmp     eax, 0Fh
00000000008EB03D  jle     short loc_8EB07C
00000000008EB03F  cmp     eax, 10h
00000000008EB042  lea     r15, [rbp-70h]
00000000008EB046  jz      short loc_8EB0C0
00000000008EB048  cmp     eax, 400h
00000000008EB04D  jz      loc_8EB1B4
00000000008EB053  cmp     eax, 1000h
00000000008EB058  jz      loc_8EB1F0
00000000008EB05E  jmp     short loc_8EB092
00000000008EB060  mov     rsi, [r14+8]
00000000008EB064  mov     rdi, r12
00000000008EB067  lea     rdx, [rbp-78h]
00000000008EB06B  call    orbis_render_context_signal_resource
00000000008EB070  or      dword ptr [rbp-8Ch], 10h
00000000008EB077  jmp     loc_8EB1F0
00000000008EB07C  cmp     eax, 4
00000000008EB07F  lea     r15, [rbp-70h]
00000000008EB083  jz      loc_8EB112
00000000008EB089  cmp     eax, 8
00000000008EB08C  jz      loc_8EB1B4
00000000008EB092  mov     eax, [r14+1Ch]
00000000008EB096  xor     r8d, r8d
00000000008EB099  cmp     eax, 0FFh
00000000008EB09E  jg      loc_8EB19F
00000000008EB0A4  cmp     eax, 10h
00000000008EB0A7  ja      loc_8EB1F0
00000000008EB0AD  mov     ecx, 10110h
00000000008EB0B2  bt      ecx, eax
00000000008EB0B5  jb      loc_8EB1B4
00000000008EB0BB  jmp     loc_8EB1F0
00000000008EB0C0  cmp     dword ptr [r14+4], 2
00000000008EB0C5  jz      short loc_8EB11D
00000000008EB0C7  mov     [rbp-0A0h], r8d
00000000008EB0CE  mov     [rbp-0B0h], rcx
00000000008EB0D5  mov     rdi, [r14+8]
00000000008EB0D9  mov     rax, [rdi]
00000000008EB0DC  call    qword ptr [rax+10h]
00000000008EB0DF  cmp     eax, 5
00000000008EB0E2  jz      loc_8EAAD8
00000000008EB0E8  cmp     eax, 1
00000000008EB0EB  jnz     loc_8EAB5C
00000000008EB0F1  mov     rdi, [r14+8]
00000000008EB0F5  call    sub_8D7240
00000000008EB0FA  vmovups ymm0, ymmword ptr [rax]
00000000008EB0FE  vmovups ymm1, ymmword ptr [rax+14h]
00000000008EB103  vmovups ymmword ptr [rbp-5Ch], ymm1
00000000008EB108  vmovups ymmword ptr [rbp-70h], ymm0
00000000008EB10D  jmp     loc_8EAC4D
00000000008EB112  cmp     dword ptr [r14+4], 2
00000000008EB117  jnz     loc_8EAA8D
00000000008EB11D  lea     rax, [r14+8]
00000000008EB121  mov     rsi, [rax]
00000000008EB124  jmp     loc_8EB1DA
00000000008EB129  mov     rdi, [r14+8]
00000000008EB12D  mov     rax, [rdi]
00000000008EB130  call    qword ptr [rax+10h]
00000000008EB133  cmp     eax, 1
00000000008EB136  jnz     short loc_8EB197
00000000008EB138  mov     rax, [r14+8]
00000000008EB13C  mov     r8d, 0
00000000008EB142  mov     rax, [rax+200h]
00000000008EB149  test    rax, rax
00000000008EB14C  jz      loc_8EB02A
00000000008EB152  lea     rcx, g_orbis_render_system
00000000008EB159  mov     rcx, [rcx]
00000000008EB15C  mov     rcx, [rcx+70h]
00000000008EB160  mov     rcx, [rcx+20h]
00000000008EB164  lea     rsi, [rax+rcx*4]
00000000008EB168  test    rsi, rsi
00000000008EB16B  jz      loc_8EB02A
00000000008EB171  imul    rax, [r12+40D90h], 0E888h
00000000008EB17D  mov     edx, 0FFFFFFFFh
00000000008EB182  mov     ecx, 3
00000000008EB187  xor     r8d, r8d
00000000008EB18A  lea     rdi, [r12+rax+5728h]
00000000008EB192  call    gnm_draw_command_buffer_wait_on_address
00000000008EB197  xor     r8d, r8d
00000000008EB19A  jmp     loc_8EB02A
00000000008EB19F  cmp     eax, 100h
00000000008EB1A4  jz      short loc_8EB1B4
00000000008EB1A6  cmp     eax, 1000h
00000000008EB1AB  jz      short loc_8EB1B4
00000000008EB1AD  cmp     eax, 400h
00000000008EB1B2  jnz     short loc_8EB1F0
00000000008EB1B4  mov     eax, [r14+4]
00000000008EB1B8  cmp     eax, 2
00000000008EB1BB  jz      short loc_8EB1D6
00000000008EB1BD  cmp     eax, 1
00000000008EB1C0  jz      loc_8EAA6E
00000000008EB1C6  test    eax, eax
00000000008EB1C8  jnz     short loc_8EB1F0
00000000008EB1CA  or      [rbp-8Ch], r8d
00000000008EB1D1  jmp     loc_8EB001
00000000008EB1D6  mov     rsi, [r14+8]
00000000008EB1DA  mov     rdi, r12
00000000008EB1DD  call    orbis_render_context_wait_for_resource
00000000008EB1E2  nop     word ptr [rax+rax+00000000h]
00000000008EB1F0  inc     r13
00000000008EB1F3  add     r14, 20h ; ' '
00000000008EB1F7  dec     rbx
00000000008EB1FA  jnz     loc_8EAF90
00000000008EB200  mov     r15, cs:qword_19A9B88
00000000008EB207  mov     ebx, [rbp-8Ch]
00000000008EB20D  test    byte ptr [rbp-0A8h], 1
00000000008EB214  jz      loc_8EB2C4
00000000008EB21A  mov     eax, [r12+4A24h]
00000000008EB222  cmp     eax, 1
00000000008EB225  jz      short loc_8EB297
00000000008EB227  test    eax, eax
00000000008EB229  jnz     loc_8EB2C4
00000000008EB22F  imul    rax, [r12+40D90h], 0E888h
00000000008EB23B  mov     r13, [r12+rax+5730h]
00000000008EB243  lea     r14, [r12+rax+5730h]
00000000008EB24B  mov     rcx, r13
00000000008EB24E  sub     rcx, [r12+rax+5738h]
00000000008EB256  shr     rcx, 2
00000000008EB25A  cmp     ecx, 2
00000000008EB25D  ja      short loc_8EB287
00000000008EB25F  mov     rdx, [r12+rax+5748h]
00000000008EB267  lea     rdi, [r12+rax+5728h]
00000000008EB26F  mov     esi, 3
00000000008EB274  call    qword ptr [r12+rax+5740h]
00000000008EB27C  test    al, al
00000000008EB27E  jz      loc_8EB343
00000000008EB284  mov     r13, [r14]
00000000008EB287  add     r13, 0FFFFFFFFFFFFFFFCh
00000000008EB28B  and     r13, 0FFFFFFFFFFFFFFFCh
00000000008EB28F  mov     [r14], r13
00000000008EB292  jmp     loc_8EB346
00000000008EB297  imul    rax, [r12+40D90h], 0F1E0h
00000000008EB2A3  imul    rcx, [r12+4A28h], 1AE0h
00000000008EB2AF  mov     esi, 7
00000000008EB2B4  add     rax, r12
00000000008EB2B7  lea     rdi, [rcx+rax+229E0h]
00000000008EB2BF  call    sub_10E05C0
00000000008EB2C4  test    ebx, ebx
00000000008EB2C6  jz      loc_8EB3B7
00000000008EB2CC  mov     eax, [r12+4A24h]
00000000008EB2D4  cmp     eax, 1
00000000008EB2D7  jz      short loc_8EB315
00000000008EB2D9  test    eax, eax
00000000008EB2DB  jnz     loc_8EB3B7
00000000008EB2E1  imul    rax, [r12+40D90h], 0E888h
00000000008EB2ED  xor     esi, esi
00000000008EB2EF  mov     edx, 1
00000000008EB2F4  xor     ecx, ecx
00000000008EB2F6  xor     r9d, r9d
00000000008EB2F9  mov     r8d, ebx
00000000008EB2FC  mov     dword ptr [rsp], 1
00000000008EB303  lea     rdi, [r12+rax+5728h]
00000000008EB30B  call    gnm_draw_command_buffer_wait_for_graphics_writes
00000000008EB310  jmp     loc_8EB3B7
00000000008EB315  imul    rax, [r12+40D90h], 0F1E0h
00000000008EB321  imul    rcx, [r12+4A28h], 1AE0h
00000000008EB32D  xor     edx, edx
00000000008EB32F  mov     esi, ebx
00000000008EB331  add     rax, r12
00000000008EB334  lea     rdi, [rcx+rax+229E0h]
00000000008EB33C  call    sub_10E0F20
00000000008EB341  jmp     short loc_8EB3B7
00000000008EB343  xor     r13d, r13d
00000000008EB346  mov     dword ptr [r13+0], 0
00000000008EB34E  or      ebx, 38h
00000000008EB351  mov     esi, 28h ; '('
00000000008EB356  xor     edx, edx
00000000008EB358  mov     r8d, 1
00000000008EB35E  mov     r9d, 1
00000000008EB364  mov     rcx, r13
00000000008EB367  imul    rax, [r12+40D90h], 0E888h
00000000008EB373  mov     [rsp], ebx
00000000008EB376  mov     dword ptr [rsp+8], 0
00000000008EB37E  lea     rdi, [r12+rax+5728h]
00000000008EB386  call    gnm_draw_command_buffer_write_release_mem_event
00000000008EB38B  imul    rax, [r12+40D90h], 0E888h
00000000008EB397  mov     edx, 0FFFFFFFFh
00000000008EB39C  mov     ecx, 3
00000000008EB3A1  mov     r8d, 1
00000000008EB3A7  mov     rsi, r13
00000000008EB3AA  lea     rdi, [r12+rax+5728h]
00000000008EB3B2  call    gnm_draw_command_buffer_wait_on_address
00000000008EB3B7  mov     rax, [r15]
00000000008EB3BA  cmp     rax, [rbp-30h]
00000000008EB3BE  jnz     short loc_8EB3D2
00000000008EB3C0  add     rsp, 0A8h
00000000008EB3C7  pop     rbx
00000000008EB3C8  pop     r12
00000000008EB3CA  pop     r13
00000000008EB3CC  pop     r14
00000000008EB3CE  pop     r15
00000000008EB3D0  pop     rbp
00000000008EB3D1  retn
00000000008EB3D2  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
