; IDA disassembly evidence for the common render particle-buffer lifecycle.
; Functions: 0x6EAFD0-0x6EB042 and 0x6ECB80-0x6ECB95

; render_create_particle_buffer at 0x6eafd0
006EAFD0  lea     rdx, g_render_system
006EAFD7  mov     rcx, rdi
006EAFDA  mov     r8, rsi
006EAFDD  mov     rsi, rcx
006EAFE0  mov     rdx, [rdx]
006EAFE3  mov     rdi, [rdx+130h]
006EAFEA  mov     rdx, [rdi]
006EAFED  mov     rax, [rdx+70h]
006EAFF1  mov     rdx, r8
006EAFF4  jmp     rax

; render_particle_buffer_construct at 0x6eb000
006EB000  lea     rax, unk_1939A20
006EB007  vxorps  xmm0, xmm0, xmm0
006EB00B  add     rax, 10h
006EB00F  mov     [rdi], rax
006EB012  mov     [rdi+8], rsi
006EB016  vmovups xmmword ptr [rdi+10h], xmm0
006EB01B  mov     qword ptr [rdi+20h], 0
006EB023  mov     dword ptr [rdi+28h], 0FFFFFFFFh
006EB02A  mov     dword ptr [rdi+2Ch], 0FFFFFFFFh
006EB031  mov     byte ptr [rdi+30h], 0
006EB035  mov     byte ptr [rdi+31h], 1
006EB039  mov     byte ptr [rdi+32h], 0
006EB03D  mov     [rdi+38h], rdx
006EB041  retn

; render_particle_buffer_destruct at 0x6ecb80
006ECB80  retn

; render_particle_buffer_delete at 0x6ecb90
006ECB90  jmp     sub_37BF50
