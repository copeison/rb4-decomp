; IDA disassembly evidence for the Orbis particle-buffer backend.
; Functions: 0x8E2D30-0x8E322B

; orbis_particle_buffer_destruct at 0x8e2d30
008E2D30  push    rbp
008E2D31  mov     rbp, rsp
008E2D34  push    r14
008E2D36  push    rbx
008E2D37  lea     rax, unk_195F780
008E2D3E  lea     r14, g_orbis_render_system
008E2D45  mov     rbx, rdi
008E2D48  add     rax, 10h
008E2D4C  mov     [rbx], rax
008E2D4F  mov     rdi, [r14]
008E2D52  mov     rsi, [rbx+140h]
008E2D59  call    orbis_defer_allocation_release
008E2D5E  mov     rdi, [r14]
008E2D61  mov     rsi, [rbx+148h]
008E2D68  call    orbis_defer_allocation_release
008E2D6D  mov     rdi, [r14]
008E2D70  mov     rsi, [rbx+160h]
008E2D77  pop     rbx
008E2D78  pop     r14
008E2D7A  pop     rbp
008E2D7B  jmp     orbis_defer_allocation_release

; orbis_particle_buffer_delete at 0x8e2d80
008E2D80  push    rbp
008E2D81  mov     rbp, rsp
008E2D84  push    r14
008E2D86  push    rbx
008E2D87  lea     rax, unk_195F780
008E2D8E  lea     r14, g_orbis_render_system
008E2D95  mov     rbx, rdi
008E2D98  add     rax, 10h
008E2D9C  mov     [rbx], rax
008E2D9F  mov     rdi, [r14]
008E2DA2  mov     rsi, [rbx+140h]
008E2DA9  call    orbis_defer_allocation_release
008E2DAE  mov     rdi, [r14]
008E2DB1  mov     rsi, [rbx+148h]
008E2DB8  call    orbis_defer_allocation_release
008E2DBD  mov     rdi, [r14]
008E2DC0  mov     rsi, [rbx+160h]
008E2DC7  call    orbis_defer_allocation_release
008E2DCC  mov     rdi, rbx
008E2DCF  pop     rbx
008E2DD0  pop     r14
008E2DD2  pop     rbp
008E2DD3  jmp     sub_37BF50

; orbis_particle_buffer_upload_vertices at 0x8e2de0
008E2DE0  mov     eax, [rdi+150h]
008E2DE6  lea     rdx, [rsi+90h]
008E2DED  not     eax
008E2DEF  and     eax, 1
008E2DF2  mov     [rdi+150h], rax
008E2DF9  mov     rcx, [rdi+rax*8+140h]
008E2E01  jmp     particle_buffer_generate_vertices; Builds four 208-byte-format vertices per active particle into the supplied platform vertex stream.

; orbis_particle_buffer_draw at 0x8e2e10
008E2E10  push    rbp
008E2E11  mov     rbp, rsp
008E2E14  push    r15
008E2E16  push    r14
008E2E18  push    r13
008E2E1A  push    r12
008E2E1C  push    rbx
008E2E1D  sub     rsp, 128h
008E2E24  mov     rbx, cs:qword_19A9B88
008E2E2B  mov     r14, rdi
008E2E2E  mov     r12, rsi
008E2E31  mov     r15, rdx
008E2E34  lea     rdx, [r12+90h]
008E2E3C  mov     rax, [rbx]
008E2E3F  mov     [rbp-30h], rax
008E2E43  mov     eax, [r14+150h]
008E2E4A  not     eax
008E2E4C  and     eax, 1
008E2E4F  mov     [r14+150h], rax
008E2E56  mov     rcx, [r14+rax*8+140h]
008E2E5E  call    particle_buffer_generate_vertices; Builds four 208-byte-format vertices per active particle into the supplied platform vertex stream.
008E2E63  cmp     qword ptr [r14+10h], 0
008E2E68  jz      loc_8E320B
008E2E6E  mov     [rbp-0F8h], r15
008E2E75  lea     r15, g_orbis_render_system
008E2E7C  lea     r13, [r14+40h]
008E2E80  xor     ebx, ebx
008E2E82  nop     word ptr [rax+rax+00000000h]
008E2E90  mov     eax, [r14+158h]
008E2E97  bt      eax, ebx
008E2E9A  jnb     short loc_8E2EB0
008E2E9C  mov     r8, [r14+150h]
008E2EA3  shl     r8, 7
008E2EA7  add     r8, r13
008E2EAA  jmp     short loc_8E2EC2
008E2EB0  mov     rax, [r15]
008E2EB3  movsxd  rcx, ebx
008E2EB6  shl     rcx, 4
008E2EBA  lea     r8, [rax+rcx+0F0Ch]
008E2EC2  imul    rax, [r12+40D90h], 0E888h
008E2ECE  mov     esi, 2
008E2ED3  mov     ecx, 1
008E2ED8  mov     edx, ebx
008E2EDA  lea     rdi, [r12+rax+5A10h]
008E2EE2  call    gnmx_cue_set_vertex_buffers
008E2EE7  inc     rbx
008E2EEA  add     r13, 10h
008E2EEE  cmp     rbx, 8
008E2EF2  jnz     short loc_8E2E90
008E2EF4  lea     rax, unk_1B5D268
008E2EFB  mov     dword ptr [rbp-0D8h], 0
008E2F05  vmovups xmm0, xmmword ptr [rax]
008E2F09  vmovups xmmword ptr [rbp-0D4h], xmm0
008E2F11  vmovups xmm0, xmmword ptr [rax]
008E2F15  lea     rax, dword_19E666C
008E2F1C  mov     edx, [rax+4]
008E2F1F  mov     esi, [rax+8]
008E2F22  mov     ecx, [rax]
008E2F24  vmovups xmmword ptr [rbp-0F0h], xmm0
008E2F2C  mov     r13d, [rax+24h]
008E2F30  mov     [rbp-13Ch], edx
008E2F36  mov     edx, [rax+0Ch]
008E2F39  mov     [rbp-144h], esi
008E2F3F  mov     esi, [rax+10h]
008E2F42  mov     [rbp-130h], ecx
008E2F48  mov     [rbp-128h], edx
008E2F4E  mov     edx, [rax+14h]
008E2F51  mov     [rbp-134h], esi
008E2F57  mov     esi, [rax+18h]
008E2F5A  mov     [rbp-140h], edx
008E2F60  mov     edx, [rax+1Ch]
008E2F63  mov     [rbp-124h], esi
008E2F69  mov     esi, [rax+20h]
008E2F6C  mov     [rbp-12Ch], edx
008E2F72  mov     rdx, [rbp-0F8h]
008E2F79  mov     [rbp-138h], esi
008E2F7F  mov     rcx, [rdx]
008E2F82  vmovups xmm0, xmmword ptr [rcx+58h]
008E2F87  vmovups xmmword ptr [rbp-0D4h], xmm0
008E2F8F  imul    rcx, [r12+40D90h], 0E888h
008E2F9B  mov     rbx, [r12+rcx+5730h]
008E2FA3  lea     r15, [r12+rcx+5730h]
008E2FAB  mov     rdx, rbx
008E2FAE  sub     rdx, [r12+rcx+5738h]
008E2FB6  shr     rdx, 2
008E2FBA  cmp     edx, 1Fh
008E2FBD  mov     edx, [rax+28h]
008E2FC0  mov     eax, [rax+2Ch]
008E2FC3  mov     [rbp-108h], eax
008E2FC9  lea     rax, unk_19E65E0
008E2FD0  mov     [rbp-0FCh], edx
008E2FD6  mov     esi, [rax+4]
008E2FD9  mov     edi, [rax+0Ch]
008E2FDC  mov     edx, [rax]
008E2FDE  mov     [rbp-118h], esi
008E2FE4  mov     esi, [rax+18h]
008E2FE7  mov     [rbp-100h], edi
008E2FED  mov     edi, [rax+10h]
008E2FF0  mov     [rbp-104h], edx
008E2FF6  mov     [rbp-10Ch], esi
008E2FFC  mov     esi, [rax+1Ch]
008E2FFF  mov     [rbp-110h], edi
008E3005  mov     edi, [rax+8]
008E3008  mov     [rbp-114h], esi
008E300E  mov     esi, [rax+14h]
008E3011  mov     eax, [rax+20h]
008E3014  mov     [rbp-11Ch], edi
008E301A  mov     [rbp-120h], esi
008E3020  mov     [rbp-0F8h], eax
008E3026  ja      short loc_8E304C
008E3028  mov     rdx, [r12+rcx+5748h]
008E3030  lea     rdi, [r12+rcx+5728h]
008E3038  mov     esi, 20h ; ' '
008E303D  call    qword ptr [r12+rcx+5740h]
008E3045  test    al, al
008E3047  jz      short loc_8E3059
008E3049  mov     rbx, [r15]
008E304C  add     rbx, 0FFFFFFFFFFFFFF88h
008E3050  and     rbx, 0FFFFFFFFFFFFFFFCh
008E3054  mov     [r15], rbx
008E3057  jmp     short loc_8E305B
008E3059  xor     ebx, ebx
008E305B  lea     r15, [rbp-0C0h]
008E3062  mov     edx, 1
008E3067  mov     rsi, rbx
008E306A  mov     rdi, r15
008E306D  call    orbis_build_instance_vertex_descriptors
008E3072  mov     eax, [rbp-130h]
008E3078  mov     esi, 2
008E307D  mov     edx, 8
008E3082  mov     ecx, 9
008E3087  mov     r8, r15
008E308A  mov     [rbx], eax
008E308C  mov     eax, [rbp-128h]
008E3092  mov     [rbx+4], eax
008E3095  mov     eax, [rbp-124h]
008E309B  mov     [rbx+8], eax
008E309E  mov     [rbx+0Ch], r13d
008E30A2  mov     eax, [rbp-13Ch]
008E30A8  mov     [rbx+10h], eax
008E30AB  mov     eax, [rbp-134h]
008E30B1  mov     [rbx+14h], eax
008E30B4  mov     eax, [rbp-12Ch]
008E30BA  mov     [rbx+18h], eax
008E30BD  mov     eax, [rbp-0FCh]
008E30C3  mov     [rbx+1Ch], eax
008E30C6  mov     eax, [rbp-144h]
008E30CC  mov     [rbx+20h], eax
008E30CF  mov     eax, [rbp-140h]
008E30D5  mov     [rbx+24h], eax
008E30D8  mov     eax, [rbp-138h]
008E30DE  mov     [rbx+28h], eax
008E30E1  mov     eax, [rbp-108h]
008E30E7  mov     [rbx+2Ch], eax
008E30EA  mov     eax, [rbp-104h]
008E30F0  mov     [rbx+30h], eax
008E30F3  mov     eax, [rbp-100h]
008E30F9  mov     [rbx+34h], eax
008E30FC  mov     eax, [rbp-10Ch]
008E3102  mov     [rbx+38h], eax
008E3105  mov     eax, [rbp-118h]
008E310B  mov     [rbx+3Ch], eax
008E310E  mov     eax, [rbp-110h]
008E3114  mov     [rbx+40h], eax
008E3117  mov     eax, [rbp-114h]
008E311D  mov     [rbx+44h], eax
008E3120  mov     eax, [rbp-11Ch]
008E3126  mov     [rbx+48h], eax
008E3129  mov     eax, [rbp-120h]
008E312F  mov     [rbx+4Ch], eax
008E3132  mov     eax, [rbp-0F8h]
008E3138  mov     [rbx+50h], eax
008E313B  mov     eax, [rbp-0C8h]
008E3141  mov     [rbx+64h], eax
008E3144  vmovups xmm0, xmmword ptr [rbp-0D8h]
008E314C  vmovups xmmword ptr [rbx+54h], xmm0
008E3151  vmovups xmm0, xmmword ptr [rbp-0F0h]
008E3159  vmovups xmmword ptr [rbx+68h], xmm0
008E315E  imul    rax, [r12+40D90h], 0E888h
008E316A  lea     rdi, [r12+rax+5A10h]
008E3172  call    gnmx_cue_set_vertex_buffers
008E3177  mov     esi, 3
008E317C  mov     rdi, r12
008E317F  call    orbis_set_primitive_type
008E3184  mov     eax, [r14+10h]
008E3188  xor     esi, esi
008E318A  mov     edx, 2
008E318F  add     eax, eax
008E3191  lea     eax, [rax+rax*2]
008E3194  mov     [rbp-0F8h], eax
008E319A  imul    rax, [r12+40D90h], 0E888h
008E31A6  lea     rdi, [r12+rax+5728h]
008E31AE  call    gnm_draw_command_buffer_set_index_size
008E31B3  imul    rax, [r12+40D90h], 0E888h
008E31BF  mov     r13, [r14+160h]
008E31C6  lea     r15, [r12+rax+5A10h]
008E31CE  lea     rbx, [r12+rax+5728h]
008E31D6  mov     rdi, r15
008E31D9  call    gnmx_prepare_draw
008E31DE  mov     esi, [rbp-0F8h]
008E31E4  xor     ecx, ecx
008E31E6  mov     rdi, rbx
008E31E9  mov     rdx, r13
008E31EC  call    gnm_draw_command_buffer_draw_index
008E31F1  mov     rdi, r15
008E31F4  call    gnmx_finish_draw
008E31F9  mov     rdi, r14
008E31FC  mov     rsi, r12
008E31FF  call    nullsub_53
008E3204  mov     rbx, cs:qword_19A9B88
008E320B  mov     rax, [rbx]
008E320E  cmp     rax, [rbp-30h]
008E3212  jnz     short loc_8E3226
008E3214  add     rsp, 128h
008E321B  pop     rbx
008E321C  pop     r12
008E321E  pop     r13
008E3220  pop     r14
008E3222  pop     r15
008E3224  pop     rbp
008E3225  retn
008E3226  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
