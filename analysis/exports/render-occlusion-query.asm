; IDA disassembly evidence for the common render occlusion-query lifecycle.
; Functions: 0x5F7D30-0x5F7DF6

; render_create_occlusion_query at 0x5f7d30
005F7D30  lea     rcx, g_render_system; Dispatches occlusion-query creation through the active render system.
005F7D37  mov     rax, rdi
005F7D3A  mov     rsi, rax
005F7D3D  mov     rcx, [rcx]
005F7D40  mov     rdi, [rcx+130h]
005F7D47  mov     rcx, [rdi]
005F7D4A  mov     rcx, [rcx+78h]
005F7D4E  jmp     rcx

; render_occlusion_query_construct at 0x5f7d50
005F7D50  lea     rax, unk_192AFD8; Constructs the 64-byte common query state and initializes its intrusive list node.
005F7D57  lea     rcx, unk_1B5D250
005F7D5E  add     rax, 10h
005F7D62  mov     [rdi], rax
005F7D65  mov     [rdi+8], rsi
005F7D69  mov     byte ptr [rdi+10h], 0
005F7D6D  lea     rax, [rdi+30h]
005F7D71  vmovups xmm0, xmmword ptr [rcx]
005F7D75  vmovups xmmword ptr [rdi+14h], xmm0
005F7D7A  mov     dword ptr [rdi+24h], 0FFFFFFFFh
005F7D81  mov     dword ptr [rdi+28h], 0
005F7D88  mov     [rdi+38h], rax
005F7D8C  mov     [rdi+30h], rax
005F7D90  mov     word ptr [rdi+11h], 0
005F7D96  retn

; render_occlusion_query_destruct at 0x5f7da0
005F7DA0  lea     rax, unk_192AFD8; Restores the common vtable and unlinks the query from its intrusive list.
005F7DA7  add     rax, 10h
005F7DAB  mov     [rdi], rax
005F7DAE  mov     rax, [rdi+30h]
005F7DB2  mov     rcx, [rdi+38h]
005F7DB6  mov     [rax+8], rcx
005F7DBA  mov     rcx, [rdi+38h]
005F7DBE  mov     [rcx], rax
005F7DC1  retn

; render_occlusion_query_delete at 0x5f7dd0
005F7DD0  lea     rax, unk_192AFD8; Deleting common occlusion-query destructor.
005F7DD7  add     rax, 10h
005F7DDB  mov     [rdi], rax
005F7DDE  mov     rax, [rdi+30h]
005F7DE2  mov     rcx, [rdi+38h]
005F7DE6  mov     [rax+8], rcx
005F7DEA  mov     rcx, [rdi+38h]
005F7DEE  mov     [rcx], rax
005F7DF1  jmp     sub_37BF50
