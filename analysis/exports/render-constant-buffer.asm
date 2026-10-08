; IDA disassembly evidence for the common render constant-buffer lifecycle.
; Functions: 0x639F30-0x63A045

; render_create_constant_buffer at 0x639f30
00639F30  push    rbp
00639F31  mov     rbp, rsp
00639F34  push    r14
00639F36  push    rbx
00639F37  mov     r14d, esi
00639F3A  lea     rsi, g_render_system
00639F41  mov     rax, rdx
00639F44  mov     rcx, rdi
00639F47  cmp     rax, 0FFFFFFFFFFFFFFFFh
00639F4B  cmovz   rax, [rcx+18h]
00639F50  mov     rdx, [rsi]
00639F53  mov     rsi, rcx
00639F56  mov     rcx, rax
00639F59  mov     rdi, [rdx+130h]
00639F60  mov     edx, r14d
00639F63  mov     rbx, [rdi]
00639F66  call    qword ptr [rbx+58h]
00639F69  mov     rbx, rax
00639F6C  test    r14b, 1
00639F70  jnz     short loc_639F85
00639F72  cmp     byte ptr [rbx+38h], 0
00639F76  jz      short loc_639F85
00639F78  mov     rax, [rbx]
00639F7B  mov     rdi, rbx
00639F7E  call    qword ptr [rax+10h]
00639F81  mov     byte ptr [rbx+38h], 0
00639F85  mov     rax, rbx
00639F88  pop     rbx
00639F89  pop     r14
00639F8B  pop     rbp
00639F8C  retn

; render_constant_buffer_construct at 0x639ff0
00639FF0  lea     rax, unk_192F070
00639FF7  add     rax, 10h
00639FFB  mov     [rdi], rax
00639FFE  mov     rax, [rsi]
0063A001  mov     [rdi+8], rax
0063A005  mov     [rdi+10h], edx
0063A008  mov     eax, [rsi+8]
0063A00B  mov     [rdi+14h], eax
0063A00E  mov     eax, [rsi+0Ch]
0063A011  mov     [rdi+18h], eax
0063A014  mov     rax, [rsi+18h]
0063A018  mov     [rdi+20h], rax
0063A01C  mov     [rdi+28h], rcx
0063A020  mov     [rdi+30h], r8
0063A024  mov     byte ptr [rdi+38h], 1
0063A028  retn

; render_constant_buffer_destruct at 0x63a030
0063A030  retn

; render_constant_buffer_delete at 0x63a040
0063A040  jmp     sub_37BF50
