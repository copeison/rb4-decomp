008D6460 cmp     dword ptr [rdi+40h], 2
008D6464 mov     rax, rsi
008D6467 lea     rsi, [rdi+138h]
008D646E jnz     short loc_8D6475
008D6470 jmp     loc_8D6480
008D6475 mov     rdx, rax
008D6478 jmp     loc_8D6850
008D647D align 20h
008D6480 push    rbp
008D6481 mov     rbp, rsp
008D6484 push    r15
008D6486 push    r14
008D6488 push    r13
008D648A push    r12
008D648C push    rbx
008D648D sub     rsp, 38h
008D6491 mov     rax, cs:qword_19A9B88
008D6498 mov     r15, rdi
008D649B mov     rax, [rax]
008D649E mov     [rbp-30h], rax
008D64A2 mov     ebx, [rsi+14h]
008D64A5 call    sub_10DEF20
008D64AA mov     edi, ebx
008D64AC mov     r14d, eax
008D64AF call    sub_8E17C0
008D64B4 mov     edi, ebx
008D64B6 mov     r12d, eax
008D64B9 call    sub_8E17F0
008D64BE mov     edi, r12d
008D64C1 mov     ebx, eax
008D64C3 call    sub_10C2C20
008D64C8 cmp     ebx, 1
008D64CB mov     edx, 3
008D64D0 lea     rsi, [rbp-34h]
008D64D4 mov     r8d, 1
008D64DA mov     edi, r14d
008D64DD mov     ecx, eax
008D64DF adc     edx, 0
008D64E2 call    sub_10F4A00
008D64E7 lea     r13, [rbp-60h]
008D64EB mov     rdi, r13
008D64EE call    sub_10C3220
008D64F3 mov     eax, [r15+70h]
008D64F7 mov     ecx, [rbp-34h]
008D64FA mov     edi, 34h ; '4'
008D64FF mov     [rbp-60h], eax
008D6502 mov     eax, [r15+74h]
008D6506 mov     [rbp-5Ch], eax
008D6509 mov     dword ptr [rbp-58h], 0
008D6510 mov     dword ptr [rbp-54h], 1
008D6517 mov     [rbp-4Ch], ebx
008D651A mov     [rbp-50h], r12d
008D651E mov     [rbp-48h], ecx
008D6521 mov     dword ptr [rbp-40h], 0
008D6528 or      byte ptr [rbp-3Ch], 1
008D652C mov     [rbp-44h], r14d
008D6530 call    sub_37BF40
008D6535 vxorps  ymm0, ymm0, ymm0
008D6539 mov     rdi, rax
008D653C mov     rsi, r13
008D653F vmovups ymmword ptr [rax+14h], ymm0
008D6544 vmovups ymmword ptr [rax], ymm0
008D6548 mov     [r15+1F8h], rax
008D654F call    sub_10C3270
008D6554 mov     rdi, [r15+1F8h]
008D655B call    sub_10C3A30
008D6560 mov     [r15+1B0h], rax
008D6567 mov     rdi, [r15+1F8h]
008D656E call    sub_10C3AE0
008D6573 mov     [r15+1B8h], rax
008D657A mov     rdi, [r15+1F8h]
008D6581 call    sub_10C3B90
008D6586 mov     edi, 50h ; 'P'
008D658B mov     [r15+1C0h], rax
008D6592 call    sub_37BF40
008D6597 lea     rcx, unk_195ED28
008D659E vxorps  ymm0, ymm0, ymm0
008D65A2 mov     dword ptr [rax+8], 1
008D65A9 mov     dword ptr [rax+0Ch], 1
008D65B0 add     rcx, 10h
008D65B4 mov     [rax], rcx
008D65B7 vmovups ymmword ptr [rax+30h], ymm0
008D65BC vmovups ymmword ptr [rax+10h], ymm0
008D65C1 mov     rbx, [r15+1D8h]
008D65C8 mov     [r15+1D8h], rax
008D65CF lea     rax, [rax+10h]
008D65D3 mov     [r15+1D0h], rax
008D65DA test    rbx, rbx
008D65DD jz      short loc_8D6620
008D65DF lea     rdi, [rbx+8]
008D65E3 mov     esi, 1
008D65E8 mov     edx, 5
008D65ED call    _Atomic_fetch_sub_4; PS4 SDK 5.008 import resolved from NID 2HnmKiLmV6s; stub: target/lib/libc_stub_weak.a
008D65F2 cmp     eax, 1
008D65F5 jnz     short loc_8D6620
008D65F7 mov     rax, [rbx]
008D65FA mov     rdi, rbx
008D65FD call    qword ptr [rax]
008D65FF lea     rdi, [rbx+0Ch]
008D6603 mov     esi, 1
008D6608 mov     edx, 5
008D660D call    _Atomic_fetch_sub_4; PS4 SDK 5.008 import resolved from NID 2HnmKiLmV6s; stub: target/lib/libc_stub_weak.a
008D6612 cmp     eax, 1
008D6615 jnz     short loc_8D6620
008D6617 mov     rax, [rbx]
008D661A mov     rdi, rbx
008D661D call    qword ptr [rax+8]
008D6620 mov     al, cs:byte_1ADEF20
008D6626 test    al, al
008D6628 jnz     short loc_8D6659
008D662A lea     rdi, byte_1ADEF20
008D6631 call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008D6636 test    eax, eax
008D6638 jz      short loc_8D6659
008D663A lea     rdi, aGpu_0; "gpu"
008D6641 call    sub_37BA70
008D6646 lea     rdi, byte_1ADEF20
008D664D mov     cs:qword_1ADEF18, rax
008D6654 call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008D6659 mov     rdi, cs:qword_1ADEF18
008D6660 call    sub_37A920
008D6665 mov     edi, [r15+1B0h]
008D666C mov     rsi, [r15+98h]
008D6673 mov     edx, [r15+1B4h]
008D667A call    sub_37AE70
008D667F mov     rcx, [r15+1D0h]
008D6686 mov     [rcx], rax
008D6689 mov     eax, [r15+1B0h]
008D6690 mov     rcx, [r15+1D0h]
008D6697 mov     [rcx+10h], rax
008D669B mov     edi, [r15+1B8h]
008D66A2 mov     rsi, [r15+98h]
008D66A9 mov     edx, [r15+1BCh]
008D66B0 call    sub_37AE70
008D66B5 mov     rcx, [r15+1D0h]
008D66BC mov     [rcx+20h], rax
008D66C0 mov     eax, [r15+1B8h]
008D66C7 mov     [rcx+28h], rax
008D66CB mov     edi, [r15+1C0h]
008D66D2 mov     rsi, [r15+98h]
008D66D9 mov     edx, [r15+1C4h]
008D66E0 call    sub_37AE70
008D66E5 mov     rcx, [r15+1D0h]
008D66EC mov     [rcx+30h], rax
008D66F0 mov     eax, [r15+1C0h]
008D66F7 mov     [rcx+38h], rax
008D66FB call    sub_37A9B0
008D6700 mov     rax, [r15+1D0h]
008D6707 mov     ebx, [r15+1B4h]
008D670E mov     r12d, [r15+1BCh]
008D6715 mov     r14, [r15+1F8h]
008D671C mov     r13, 0FFFFFFFFFFh
008D6726 mov     rcx, [rax]
008D6729 mov     rax, [rax+20h]
008D672D mov     rdi, r14
008D6730 add     rcx, rbx
008D6733 neg     rbx
008D6736 add     rax, r12
008D6739 neg     r12
008D673C add     rcx, r13
008D673F add     rax, r13
008D6742 and     rbx, rcx
008D6745 and     r12, rax
008D6748 shr     rbx, 8
008D674C mov     esi, ebx
008D674E call    sub_10C48C0
008D6753 mov     rdi, r14
008D6756 mov     esi, ebx
008D6758 call    sub_10C4B80
008D675D shr     r12, 8
008D6761 mov     rdi, r14
008D6764 mov     esi, r12d
008D6767 call    sub_10C4E40
008D676C mov     rdi, r14
008D676F mov     esi, r12d
008D6772 call    sub_10C5130
008D6777 mov     rax, [r15+1D0h]
008D677E mov     ecx, [r15+1C4h]
008D6785 mov     rbx, [r15+1F8h]
008D678C mov     edi, 20h ; ' '
008D6791 mov     rax, [rax+30h]
008D6795 add     rax, rcx
008D6798 neg     rcx
008D679B add     rax, r13
008D679E and     rcx, rax
008D67A1 mov     eax, 5FFFFFFFh
008D67A6 shr     rcx, 8
008D67AA mov     [rbx+24h], ecx
008D67AD and     eax, [rbx]
008D67AF or      eax, 20000000h
008D67B4 mov     [rbx], eax
008D67B6 call    sub_37BF40
008D67BB vxorps  ymm0, ymm0, ymm0
008D67BF xor     edx, edx
008D67C1 mov     rdi, rax
008D67C4 mov     rsi, rbx
008D67C7 vmovups ymmword ptr [rax], ymm0
008D67CB mov     [r15+198h], rax
008D67D2 call    sub_10DE1C0
008D67D7 mov     rdi, [r15+198h]
008D67DE mov     esi, 6Dh ; 'm'
008D67E3 call    sub_10DEA40
008D67E8 mov     edi, 20h ; ' '
008D67ED call    sub_37BF40
008D67F2 vxorps  ymm0, ymm0, ymm0
008D67F6 mov     edx, 4
008D67FB xor     ecx, ecx
008D67FD mov     rdi, rax
008D6800 vmovups ymmword ptr [rax], ymm0
008D6804 mov     [r15+1A8h], rax
008D680B mov     rsi, [r15+1F8h]
008D6812 call    sub_10DE490
008D6817 mov     rdi, [r15+1A8h]
008D681E mov     esi, 6Dh ; 'm'
008D6823 call    sub_10DEA40
008D6828 mov     rax, cs:qword_19A9B88
008D682F mov     rax, [rax]
008D6832 cmp     rax, [rbp-30h]
008D6836 jnz     short loc_8D6847
008D6838 add     rsp, 38h
008D683C pop     rbx
008D683D pop     r12
008D683F pop     r13
008D6841 pop     r14
008D6843 pop     r15
008D6845 pop     rbp
008D6846 retn
008D6847 call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
008D684C align 10h
008D6850 push    rbp
008D6851 mov     rbp, rsp
008D6854 push    r15
008D6856 push    r14
008D6858 push    r13
008D685A push    r12
008D685C push    rbx
008D685D sub     rsp, 98h
008D6864 mov     rax, cs:qword_19A9B88
008D686B mov     [rbp-0B0h], rsi
008D6872 mov     r13, rdi
008D6875 mov     r15, rdx
008D6878 lea     r12, [r13+40h]
008D687C mov     rax, [rax]
008D687F mov     [rbp-30h], rax
008D6883 mov     ebx, [rsi+14h]
008D6886 call    sub_10DEF20
008D688B mov     rdi, r12
008D688E mov     r14d, eax
008D6891 call    sub_8E1820
008D6896 mov     edi, ebx
008D6898 mov     r12d, eax
008D689B call    sub_8E1790
008D68A0 lea     rsi, [rbp-34h]
008D68A4 mov     r8d, 1
008D68AA mov     edi, r14d
008D68AD mov     edx, r12d
008D68B0 mov     ecx, eax
008D68B2 call    sub_10F4A00
008D68B7 mov     eax, [r13+68h]
008D68BB lea     r12, [rbp-68h]
008D68BF mov     rdi, r12
008D68C2 and     eax, 1
008D68C5 inc     eax
008D68C7 mov     [rbp-0B8h], rax
008D68CE call    sub_10DC780
008D68D3 mov     dword ptr [rbp-68h], 9
008D68DA mov     edi, ebx
008D68DC mov     eax, [r13+70h]
008D68E0 mov     [rbp-64h], eax
008D68E3 mov     eax, [r13+74h]
008D68E7 mov     [rbp-60h], eax
008D68EA mov     dword ptr [rbp-5Ch], 1
008D68F1 mov     dword ptr [rbp-58h], 0
008D68F8 mov     dword ptr [rbp-50h], 1
008D68FF call    sub_8E1790
008D6904 mov     rdi, [rbp-0B0h]
008D690B mov     ecx, [rbp-34h]
008D690E mov     [rbp-4Ch], eax
008D6911 mov     [rbp-48h], ecx
008D6914 mov     dword ptr [rbp-40h], 0
008D691B call    sub_6832D0
008D6920 inc     eax
008D6922 xor     ebx, ebx
008D6924 mov     [rbp-54h], eax
008D6927 mov     [rbp-44h], r14d
008D692B nop     dword ptr [rax+rax+00h]
008D6930 mov     edi, 20h ; ' '
008D6935 call    sub_37BF40
008D693A vxorps  ymm0, ymm0, ymm0
008D693E mov     rdi, rax
008D6941 mov     rsi, r12
008D6944 vmovups ymmword ptr [rax], ymm0
008D6948 mov     [r13+rbx*8+198h], rax
008D6950 call    sub_10DC7A0
008D6955 mov     rdi, [r13+rbx*8+198h]
008D695D call    sub_10DCD20
008D6962 test    r15, r15
008D6965 mov     [r13+1B0h], rax
008D696C jz      short loc_8D69B0
008D696E mov     rdx, [r15+1D0h]
008D6975 mov     rcx, rax
008D6978 mov     eax, eax
008D697A shr     rcx, 20h
008D697E mov     rsi, [rdx+rbx*8]
008D6982 lea     rdi, [rsi+rcx-1]
008D6987 add     rsi, [rdx+rbx*8+10h]
008D698C neg     rcx
008D698F and     rcx, rdi
008D6992 add     rax, rcx
008D6995 cmp     rax, rsi
008D6998 mov     eax, 0
008D699D cmova   r15, rax
008D69A1 jmp     short loc_8D69B3
008D69A3 align 10h
008D69B0 xor     r15d, r15d
008D69B3 inc     rbx
008D69B6 cmp     [rbp-0B8h], rbx
008D69BD jnz     loc_8D6930
008D69C3 test    r15, r15
008D69C6 jz      short loc_8D69F1
008D69C8 mov     r12, [r15+1D8h]
008D69CF mov     rbx, [r15+1D0h]
008D69D6 test    r12, r12
008D69D9 jz      short loc_8D6A36
008D69DB lea     rdi, [r12+8]
008D69E0 mov     esi, 1
008D69E5 mov     edx, 5
008D69EA call    _Atomic_fetch_add_4; PS4 SDK 5.008 import resolved from NID iPBqs+YUUFw; stub: target/lib/libc_stub_weak.a
008D69EF jmp     short loc_8D6A36
008D69F1 mov     edi, 50h ; 'P'
008D69F6 call    sub_37BF40
008D69FB lea     rcx, unk_195ED28
008D6A02 mov     r12, rax
008D6A05 vxorps  ymm0, ymm0, ymm0
008D6A09 lea     rbx, [r12+10h]
008D6A0E mov     dword ptr [r12+8], 1
008D6A17 mov     dword ptr [r12+0Ch], 1
008D6A20 add     rcx, 10h
008D6A24 mov     [r12], rcx
008D6A28 vmovups ymmword ptr [r12+30h], ymm0
008D6A2F vmovups ymmword ptr [r12+10h], ymm0
008D6A36 mov     r15, [r13+1D8h]
008D6A3D mov     [r13+1D8h], r12
008D6A44 mov     [r13+1D0h], rbx
008D6A4B test    r15, r15
008D6A4E jz      short loc_8D6A91
008D6A50 lea     rdi, [r15+8]
008D6A54 mov     esi, 1
008D6A59 mov     edx, 5
008D6A5E call    _Atomic_fetch_sub_4; PS4 SDK 5.008 import resolved from NID 2HnmKiLmV6s; stub: target/lib/libc_stub_weak.a
008D6A63 cmp     eax, 1
008D6A66 jnz     short loc_8D6A91
008D6A68 mov     rax, [r15]
008D6A6B mov     rdi, r15
008D6A6E call    qword ptr [rax]
008D6A70 lea     rdi, [r15+0Ch]
008D6A74 mov     esi, 1
008D6A79 mov     edx, 5
008D6A7E call    _Atomic_fetch_sub_4; PS4 SDK 5.008 import resolved from NID 2HnmKiLmV6s; stub: target/lib/libc_stub_weak.a
008D6A83 cmp     eax, 1
008D6A86 jnz     short loc_8D6A91
008D6A88 mov     rax, [r15]
008D6A8B mov     rdi, r15
008D6A8E call    qword ptr [rax+8]
008D6A91 lea     r14, [rbp-98h]
008D6A98 xor     r12d, r12d
008D6A9B nop     dword ptr [rax+rax+00h]
008D6AA0 mov     rax, [r13+1D0h]
008D6AA7 cmp     qword ptr [rax+r12*8], 0
008D6AAC jnz     loc_8D6B3B
008D6AB2 mov     al, cs:byte_1ADEF10
008D6AB8 test    al, al
008D6ABA jnz     short loc_8D6AEB
008D6ABC lea     rdi, byte_1ADEF10
008D6AC3 call    __cxa_guard_acquire; PS4 SDK 5.008 import resolved from NID 3GPpjQdAMTw; stub: target/lib/libc_stub_weak.a
008D6AC8 test    eax, eax
008D6ACA jz      short loc_8D6AEB
008D6ACC lea     rdi, aGpu_0; "gpu"
008D6AD3 call    sub_37BA70
008D6AD8 lea     rdi, byte_1ADEF10
008D6ADF mov     cs:qword_1ADEF08, rax
008D6AE6 call    __cxa_guard_release; PS4 SDK 5.008 import resolved from NID 9rAeANT2tyE; stub: target/lib/libc_stub_weak.a
008D6AEB mov     rdi, cs:qword_1ADEF08
008D6AF2 call    sub_37A920
008D6AF7 mov     edi, [r13+1B0h]
008D6AFE mov     rsi, [r13+98h]
008D6B05 mov     edx, [r13+1B4h]
008D6B0C call    sub_37AE70
008D6B11 mov     rcx, [r13+1D0h]
008D6B18 mov     [rcx+r12*8], rax
008D6B1C mov     eax, [r13+1B0h]
008D6B23 mov     rcx, [r13+1D0h]
008D6B2A mov     [rcx+r12*8+10h], rax
008D6B2F call    sub_37A9B0
008D6B34 mov     rax, [r13+1D0h]
008D6B3B mov     rax, [rax+r12*8]
008D6B3F mov     edx, [r13+1B4h]
008D6B46 mov     rcx, r12
008D6B49 lea     rax, [rax+rdx-1]
008D6B4E neg     rdx
008D6B51 and     rdx, rax
008D6B54 mov     [rbp-0C0h], rdx
008D6B5B mov     rdx, [rbp-0B0h]
008D6B62 cmp     qword ptr [rdx+18h], 0
008D6B67 jz      loc_8D6C0E
008D6B6D mov     rdi, [rbp-0B0h]
008D6B74 mov     r12, rcx
008D6B77 call    sub_6832D0
008D6B7C mov     rcx, r12
008D6B7F cmp     rax, 0FFFFFFFFFFFFFFFFh
008D6B83 jz      loc_8D6C0E
008D6B89 mov     rbx, [rbp-0B0h]
008D6B90 xor     r15d, r15d
008D6B93 nop     word ptr [rax+rax+00000000h]
008D6BA0 mov     rsi, [r13+rcx*8+198h]
008D6BA8 xor     ecx, ecx
008D6BAA mov     rdi, r14
008D6BAD mov     edx, r15d
008D6BB0 call    sub_10FC210
008D6BB5 mov     rdx, [r13+r12*8+198h]
008D6BBD xor     r8d, r8d
008D6BC0 lea     rdi, [rbp-0A0h]
008D6BC7 lea     rsi, [rbp-0A8h]
008D6BCE mov     ecx, r15d
008D6BD1 call    sub_10FD200
008D6BD6 mov     rdi, [rbp-0A0h]
008D6BDD mov     rsi, [rbx+18h]
008D6BE1 mov     rdx, r14
008D6BE4 add     rdi, [rbp-0C0h]
008D6BEB call    sub_10FDB10
008D6BF0 mov     rdi, [rbp-0B0h]
008D6BF7 mov     rbx, [rbx+28h]
008D6BFB inc     r15
008D6BFE call    sub_6832D0
008D6C03 inc     rax
008D6C06 mov     rcx, r12
008D6C09 cmp     r15, rax
008D6C0C jb      short loc_8D6BA0
008D6C0E mov     rsi, [rbp-0C0h]
008D6C15 mov     rdi, [r13+rcx*8+198h]
008D6C1D mov     r12, rcx
008D6C20 shr     rsi, 8
008D6C24 call    sub_10DDED0
008D6C29 mov     eax, [r13+68h]
008D6C2D mov     ecx, eax
008D6C2F and     ecx, 12h
008D6C32 cmp     ecx, 2
008D6C35 jnz     short loc_8D6CA0
008D6C37 mov     edi, 40h ; '@'
008D6C3C call    sub_37BF40
008D6C41 vxorps  ymm0, ymm0, ymm0
008D6C45 mov     ecx, 410h
008D6C4A mov     rdi, rax
008D6C4D vmovups ymmword ptr [rax+20h], ymm0
008D6C52 vmovups ymmword ptr [rax], ymm0
008D6C56 mov     [r13+r12*8+1E8h], rax
008D6C5E mov     rsi, [r13+r12*8+198h]
008D6C66 mov     edx, [rsi+0Ch]
008D6C69 bextr   ecx, edx, ecx
008D6C6E cmp     edx, 0DFFFFFFFh
008D6C74 mov     edx, 0
008D6C79 cmovbe  ecx, edx
008D6C7C xor     edx, edx
008D6C7E xor     r8d, r8d
008D6C81 xor     r9d, r9d
008D6C84 call    sub_10DAA70
008D6C89 mov     rdi, [r13+r12*8+198h]
008D6C91 jmp     short loc_8D6CC0
008D6C93 align 20h
008D6CA0 mov     rdi, [r13+r12*8+198h]
008D6CA8 test    al, 10h
008D6CAA jnz     short loc_8D6CC0
008D6CAC mov     esi, 10h
008D6CB1 jmp     short loc_8D6CC5
008D6CB3 align 20h
008D6CC0 mov     esi, 6Dh ; 'm'
008D6CC5 call    sub_10DEA40
008D6CCA inc     r12
008D6CCD cmp     r12, [rbp-0B8h]
008D6CD4 jnz     loc_8D6AA0
008D6CDA mov     rax, cs:qword_19A9B88
008D6CE1 mov     rax, [rax]
008D6CE4 cmp     rax, [rbp-30h]
008D6CE8 jnz     short loc_8D6CFC
008D6CEA add     rsp, 98h
008D6CF1 pop     rbx
008D6CF2 pop     r12
008D6CF4 pop     r13
008D6CF6 pop     r14
008D6CF8 pop     r15
008D6CFA pop     rbp
008D6CFB retn
008D6CFC call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
