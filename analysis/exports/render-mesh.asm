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
