; render_mesh_construct @ 0x5C2700
005C2700  push    rbp; Constructs the exact 128-byte common mesh base.
005C2701  mov     rbp, rsp
005C2704  push    r14
005C2706  push    rbx
005C2707  lea     rax, unk_1928878
005C270E  mov     rbx, rdi
005C2711  mov     r14, rsi
005C2714  lea     rdi, [rbx+8]
005C2718  add     rax, 10h
005C271C  mov     [rbx], rax
005C271F  call    sub_6C9BA0
005C2724  lea     rax, unk_19287E0
005C272B  lea     rdi, [rbx+30h]
005C272F  lea     rsi, aEastlVector_208; "EASTL vector"
005C2736  mov     rcx, rax
005C2739  add     rax, 10h
005C273D  sub     rcx, 0FFFFFFFFFFFFFF80h
005C2741  vmovq   xmm1, rax
005C2746  vmovq   xmm0, rcx
005C274B  vpunpcklqdq xmm0, xmm1, xmm0
005C274F  vmovdqu xmmword ptr [rbx], xmm0
005C2753  vpxor   xmm0, xmm0, xmm0
005C2757  vmovdqu xmmword ptr [rbx+18h], xmm0
005C275C  mov     qword ptr [rbx+28h], 0
005C2764  call    nullsub_18
005C2769  lea     rax, unk_1B5D250
005C2770  vpxor   xmm0, xmm0, xmm0
005C2774  vmovdqu xmmword ptr [rbx+38h], xmm0
005C2779  mov     qword ptr [rbx+48h], 0FFFFFFFFFFFFFFFFh
005C2781  mov     byte ptr [rbx+50h], 0
005C2785  mov     byte ptr [rbx+51h], 0
005C2789  mov     qword ptr [rbx+54h], 0
005C2791  vmovups xmm0, xmmword ptr [rax]
005C2795  xor     eax, eax
005C2797  vmovups xmmword ptr [rbx+5Ch], xmm0
005C279C  xchg    eax, [rbx+6Ch]
005C279F  mov     qword ptr [rbx+70h], 0FFFFFFFFFFFFFFFFh
005C27A7  mov     [rbx+78h], r14
005C27AB  pop     rbx
005C27AC  pop     r14
005C27AE  pop     rbp
005C27AF  retn

; render_mesh_destruct @ 0x5C27B0
005C27B0  push    rbp; Destroys the triangle array and update-listener link in the common mesh base.
005C27B1  mov     rbp, rsp
005C27B4  push    rbx
005C27B5  push    rax
005C27B6  lea     rax, unk_19287E0
005C27BD  lea     rbx, [rdi+8]
005C27C1  mov     rcx, rax
005C27C4  add     rax, 10h
005C27C8  sub     rcx, 0FFFFFFFFFFFFFF80h
005C27CC  vmovq   xmm1, rax
005C27D1  vmovq   xmm0, rcx
005C27D6  vpunpcklqdq xmm0, xmm1, xmm0
005C27DA  vmovdqu xmmword ptr [rdi], xmm0
005C27DE  mov     rsi, [rdi+18h]
005C27E2  test    rsi, rsi
005C27E5  jz      short loc_5C27F7
005C27E7  mov     rdx, [rdi+28h]
005C27EB  add     rdi, 30h ; '0'
005C27EF  sub     rdx, rsi
005C27F2  call    sub_252D30
005C27F7  mov     rdi, rbx
005C27FA  add     rsp, 8
005C27FE  pop     rbx
005C27FF  pop     rbp
005C2800  jmp     sub_6C9BC0

; render_mesh_secondary_destruct @ 0x5C2810
005C2810  push    rbp; Secondary-base destructor entry; adjusts the update-link this pointer by -8.
005C2811  mov     rbp, rsp
005C2814  push    rbx
005C2815  push    rax
005C2816  lea     rax, unk_19287E0
005C281D  mov     rbx, rdi
005C2820  mov     rcx, rax
005C2823  add     rax, 10h
005C2827  sub     rcx, 0FFFFFFFFFFFFFF80h
005C282B  vmovq   xmm1, rax
005C2830  vmovq   xmm0, rcx
005C2835  vpunpcklqdq xmm0, xmm1, xmm0
005C2839  vmovdqu xmmword ptr [rbx-8], xmm0
005C283E  mov     rsi, [rbx+10h]
005C2842  test    rsi, rsi
005C2845  jz      short loc_5C285E
005C2847  mov     rdi, rbx
005C284A  add     rdi, 0FFFFFFFFFFFFFFF8h
005C284E  mov     rdx, [rdi+28h]
005C2852  add     rdi, 30h ; '0'
005C2856  sub     rdx, rsi
005C2859  call    sub_252D30
005C285E  mov     rdi, rbx
005C2861  add     rsp, 8
005C2865  pop     rbx
005C2866  pop     rbp
005C2867  jmp     sub_6C9BC0

; render_mesh_delete @ 0x5C2870
005C2870  push    rbp; Deleting destructor for the common RenderMesh object.
005C2871  mov     rbp, rsp
005C2874  push    r14
005C2876  push    rbx
005C2877  lea     rax, unk_19287E0
005C287E  mov     rbx, rdi
005C2881  lea     r14, [rbx+8]
005C2885  mov     rcx, rax
005C2888  add     rax, 10h
005C288C  sub     rcx, 0FFFFFFFFFFFFFF80h
005C2890  vmovq   xmm1, rax
005C2895  vmovq   xmm0, rcx
005C289A  vpunpcklqdq xmm0, xmm1, xmm0
005C289E  vmovdqu xmmword ptr [rbx], xmm0
005C28A2  mov     rsi, [rbx+18h]
005C28A6  test    rsi, rsi
005C28A9  jz      short loc_5C28BB
005C28AB  mov     rdx, [rbx+28h]
005C28AF  lea     rdi, [rbx+30h]
005C28B3  sub     rdx, rsi
005C28B6  call    sub_252D30
005C28BB  mov     rdi, r14
005C28BE  call    sub_6C9BC0
005C28C3  mov     rdi, rbx
005C28C6  pop     rbx
005C28C7  pop     r14
005C28C9  pop     rbp
005C28CA  jmp     sub_37BF50

; render_mesh_secondary_delete @ 0x5C28D0
005C28D0  push    rbp; Secondary-base deleting destructor entry; adjusts this by -8.
005C28D1  mov     rbp, rsp
005C28D4  push    r14
005C28D6  push    rbx
005C28D7  lea     rax, unk_19287E0
005C28DE  mov     rbx, rdi
005C28E1  lea     r14, [rbx-8]
005C28E5  mov     rcx, rax
005C28E8  add     rax, 10h
005C28EC  sub     rcx, 0FFFFFFFFFFFFFF80h
005C28F0  vmovq   xmm1, rax
005C28F5  vmovq   xmm0, rcx
005C28FA  vpunpcklqdq xmm0, xmm1, xmm0
005C28FE  vmovdqu xmmword ptr [rbx-8], xmm0
005C2903  mov     rsi, [rbx+10h]
005C2907  test    rsi, rsi
005C290A  jz      short loc_5C291C
005C290C  mov     rdx, [r14+28h]
005C2910  lea     rdi, [r14+30h]
005C2914  sub     rdx, rsi
005C2917  call    sub_252D30
005C291C  mov     rdi, rbx
005C291F  call    sub_6C9BC0
005C2924  mov     rdi, r14
005C2927  pop     rbx
005C2928  pop     r14
005C292A  pop     rbp
005C292B  jmp     sub_37BF50

; render_mesh_set_vertices_resident @ 0x5C2930
005C2930  mov     [rdi+50h], sil
005C2934  retn

; render_mesh_set_vertex_usage_flags @ 0x5C2940
005C2940  mov     [rdi+54h], esi
005C2943  retn

; render_mesh_set_triangle_usage_flags @ 0x5C2950
005C2950  mov     [rdi+58h], esi
005C2953  retn

; render_mesh_requires_vertex_storage @ 0x5C2960
005C2960  cmp     byte ptr [rdi+50h], 0; Returns whether the current residency and usage flags require CPU vertex storage.
005C2964  mov     al, 1
005C2966  jnz     short locret_5C2975
005C2968  cmp     byte ptr [rdi+51h], 0
005C296C  jnz     short locret_5C2975
005C296E  test    byte ptr [rdi+54h], 5
005C2972  setnz   al
005C2975  retn

; render_mesh_requires_triangle_storage @ 0x5C2980
005C2980  cmp     byte ptr [rdi+50h], 0; Returns whether the current residency and usage flags require CPU triangle storage.
005C2984  mov     al, 1
005C2986  jnz     short locret_5C2995
005C2988  cmp     byte ptr [rdi+51h], 0
005C298C  jnz     short locret_5C2995
005C298E  test    byte ptr [rdi+58h], 5
005C2992  setnz   al
005C2995  retn

; render_mesh_finalize @ 0x5C29A0
005C29A0  push    rbp; Finalizes backend mesh state and conditionally releases CPU vertex and triangle storage.
005C29A1  mov     rbp, rsp
005C29A4  push    r15
005C29A6  push    r14
005C29A8  push    rbx
005C29A9  sub     rsp, 28h
005C29AD  mov     r15, cs:qword_19A9B88
005C29B4  mov     rbx, rdi
005C29B7  mov     rcx, 0AAAAAAAAAAAAAAABh
005C29C1  mov     rax, [r15]
005C29C4  mov     [rbp+var_20], rax
005C29C8  mov     rax, [rbx+20h]
005C29CC  sub     rax, [rbx+18h]
005C29D0  sar     rax, 2
005C29D4  imul    rcx, rax
005C29D8  mov     [rbx+40h], rcx
005C29DC  mov     rax, [rbx]
005C29DF  call    qword ptr [rax+48h]
005C29E2  cmp     byte ptr [rbx+50h], 0
005C29E6  jnz     short loc_5C2A5D
005C29E8  cmp     byte ptr [rbx+51h], 0
005C29EC  jnz     short loc_5C2A5D
005C29EE  test    byte ptr [rbx+54h], 5
005C29F2  jz      short loc_5C2A4E
005C29F4  cmp     byte ptr [rbx+51h], 0
005C29F8  jnz     short loc_5C2A5D
005C29FA  test    byte ptr [rbx+58h], 5
005C29FE  jnz     short loc_5C2A5D
005C2A00  lea     r14, [rbp+var_28]
005C2A04  vxorps  xmm0, xmm0, xmm0
005C2A08  lea     rsi, aEastlVector_208; "EASTL vector"
005C2A0F  add     rbx, 18h
005C2A13  mov     rdi, r14
005C2A16  vmovups [rbp+var_40], xmm0
005C2A1B  mov     [rbp+var_30], 0
005C2A23  call    nullsub_18
005C2A28  lea     rsi, [rbp+var_40]
005C2A2C  mov     rdi, rbx
005C2A2F  call    sub_5C2A80
005C2A34  mov     rsi, qword ptr [rbp+var_40]
005C2A38  test    rsi, rsi
005C2A3B  jz      short loc_5C2A5D
005C2A3D  mov     rdx, [rbp+var_30]
005C2A41  mov     rdi, r14
005C2A44  sub     rdx, rsi
005C2A47  call    sub_252D30
005C2A4C  jmp     short loc_5C2A5D
005C2A4E  mov     rax, [rbx]
005C2A51  mov     rdi, rbx
005C2A54  call    qword ptr [rax+30h]
005C2A57  cmp     byte ptr [rbx+50h], 0
005C2A5B  jz      short loc_5C29F4
005C2A5D  mov     rax, [r15]
005C2A60  cmp     rax, [rbp+var_20]
005C2A64  jnz     short loc_5C2A71
005C2A66  add     rsp, 28h
005C2A6A  pop     rbx
005C2A6B  pop     r14
005C2A6D  pop     r15
005C2A6F  pop     rbp
005C2A70  retn
005C2A71  call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a

; render_mesh_process_pending_updates @ 0x5C2E40
005C2E40  push    rbp; Processes the atomic dirty mask, refreshes triangle count, dispatches the backend update, and clears the mask.
005C2E41  mov     rbp, rsp
005C2E44  push    rbx
005C2E45  push    rax
005C2E46  mov     rbx, rdi
005C2E49  mov     eax, [rbx+6Ch]
005C2E4C  test    eax, eax
005C2E4E  jz      short loc_5C2E99
005C2E50  lea     rax, g_render_system
005C2E57  mov     edx, [rbx+6Ch]
005C2E5A  mov     rax, [rax]
005C2E5D  test    dl, 2
005C2E60  mov     rax, [rax+0A0h]
005C2E67  mov     [rbx+70h], rax
005C2E6B  jz      short loc_5C2E8B
005C2E6D  mov     rax, [rbx+20h]
005C2E71  mov     rcx, 0AAAAAAAAAAAAAAABh
005C2E7B  sub     rax, [rbx+18h]
005C2E7F  sar     rax, 2
005C2E83  imul    rcx, rax
005C2E87  mov     [rbx+40h], rcx
005C2E8B  mov     rax, [rbx]
005C2E8E  mov     rdi, rbx
005C2E91  call    qword ptr [rax+50h]
005C2E94  xor     eax, eax
005C2E96  xchg    eax, [rbx+6Ch]
005C2E99  add     rsp, 8
005C2E9D  pop     rbx
005C2E9E  pop     rbp
005C2E9F  retn

; render_mesh_process_pending_updates_secondary @ 0x5C2EA0
005C2EA0  push    rbp
005C2EA1  mov     rbp, rsp
005C2EA4  push    rbx
005C2EA5  push    rax
005C2EA6  mov     rbx, rdi
005C2EA9  mov     eax, [rbx+64h]
005C2EAC  test    eax, eax
005C2EAE  jz      short loc_5C2EFD
005C2EB0  lea     rax, g_render_system
005C2EB7  add     rbx, 0FFFFFFFFFFFFFFF8h
005C2EBB  mov     edx, [rbx+6Ch]
005C2EBE  mov     rax, [rax]
005C2EC1  test    dl, 2
005C2EC4  mov     rax, [rax+0A0h]
005C2ECB  mov     [rbx+70h], rax
005C2ECF  jz      short loc_5C2EEF
005C2ED1  mov     rax, [rbx+20h]
005C2ED5  mov     rcx, 0AAAAAAAAAAAAAAABh
005C2EDF  sub     rax, [rbx+18h]
005C2EE3  sar     rax, 2
005C2EE7  imul    rcx, rax
005C2EEB  mov     [rbx+40h], rcx
005C2EEF  mov     rax, [rbx]
005C2EF2  mov     rdi, rbx
005C2EF5  call    qword ptr [rax+50h]
005C2EF8  xor     eax, eax
005C2EFA  xchg    eax, [rbx+6Ch]
005C2EFD  add     rsp, 8
005C2F01  pop     rbx
005C2F02  pop     rbp
005C2F03  retn
