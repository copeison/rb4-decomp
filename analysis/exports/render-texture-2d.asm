; render_texture_2d_construct @ 0x6900B0
006900B0  push    rbp; Constructs the exact 408-byte common 2D texture object.
006900B1  mov     rbp, rsp
006900B4  push    r14
006900B6  push    rbx
006900B7  mov     r14, rsi
006900BA  mov     rbx, rdi
006900BD  call    render_texture_construct
006900C2  lea     rax, unk_19349C0
006900C9  mov     rdi, r14
006900CC  add     rax, 10h
006900D0  mov     [rbx], rax
006900D3  call    sub_69B990
006900D8  vmovups ymm0, ymmword ptr [r14+70h]
006900DE  lea     rdi, [rbx+138h]
006900E5  lea     rsi, [r14+90h]
006900EC  movzx   edx, al
006900EF  vmovups ymmword ptr [rbx+118h], ymm0
006900F7  vmovups ymm0, ymmword ptr [r14]
006900FC  vmovups ymm1, ymmword ptr [r14+20h]
00690102  vmovups ymm2, ymmword ptr [r14+40h]
00690108  vmovups ymm3, ymmword ptr [r14+60h]
0069010E  vmovups ymmword ptr [rbx+108h], ymm3
00690116  vmovups ymmword ptr [rbx+0E8h], ymm2
0069011E  vmovups ymmword ptr [rbx+0C8h], ymm1
00690126  vmovups ymmword ptr [rbx+0A8h], ymm0
0069012E  call    sub_682960
00690133  mov     qword ptr [rbx+188h], 0
0069013E  mov     qword ptr [rbx+190h], 0FFFFFFFFFFFFFFFFh
00690149  mov     dword ptr [rbx+0A0h], 0
00690153  mov     eax, [r14+0A4h]
0069015A  mov     [rbx+104h], eax
00690160  mov     eax, [r14+0A0h]
00690167  mov     [rbx+110h], eax
0069016D  mov     rax, [r14+98h]
00690174  mov     [rbx+108h], rax
0069017B  vmovups ymm0, ymmword ptr [rbx+118h]
00690183  vmovups ymmword ptr [rbx+80h], ymm0
0069018B  vmovups ymm0, ymmword ptr [rbx+0A8h]
00690193  vmovups ymm1, ymmword ptr [rbx+0C8h]
0069019B  vmovups ymm2, ymmword ptr [rbx+0E8h]
006901A3  vmovups ymm3, ymmword ptr [rbx+108h]
006901AB  vmovups ymmword ptr [rbx+70h], ymm3
006901B0  vmovups ymmword ptr [rbx+50h], ymm2
006901B5  vmovups ymmword ptr [rbx+30h], ymm1
006901BA  vmovups ymmword ptr [rbx+10h], ymm0
006901BF  pop     rbx
006901C0  pop     r14
006901C2  pop     rbp
006901C3  retn

; render_texture_2d_destruct @ 0x6901D0
006901D0  push    rbp; Destroys the common 2D mip-chain state and RenderTexture base.
006901D1  mov     rbp, rsp
006901D4  push    rbx
006901D5  push    rax
006901D6  lea     rax, unk_19349C0
006901DD  mov     rbx, rdi
006901E0  lea     rdi, [rbx+138h]
006901E7  add     rax, 10h
006901EB  mov     [rbx], rax
006901EE  call    sub_682BC0
006901F3  mov     rdi, rbx
006901F6  add     rsp, 8
006901FA  pop     rbx
006901FB  pop     rbp
006901FC  jmp     render_texture_destruct

; render_texture_2d_delete @ 0x690210
00690210  push    rbp; Deleting destructor for the common RenderTexture2D object.
00690211  mov     rbp, rsp
00690214  push    rbx
00690215  push    rax
00690216  lea     rax, unk_19349C0
0069021D  mov     rbx, rdi
00690220  lea     rdi, [rbx+138h]
00690227  add     rax, 10h
0069022B  mov     [rbx], rax
0069022E  call    sub_682BC0
00690233  mov     rdi, rbx
00690236  call    render_texture_destruct
0069023B  mov     rdi, rbx
0069023E  add     rsp, 8
00690242  pop     rbx
00690243  pop     rbp
00690244  jmp     sub_37BF50

; render_texture_2d_set_linked_resource @ 0x690250
00690250  mov     [rdi+188h], rsi; Stores the optional linked resource pointer and signed index.
00690257  mov     [rdi+190h], rdx
0069025E  retn
