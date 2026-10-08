; IDA disassembly evidence for the Orbis GPU fence lifecycle and commands.
; Functions: 0x8D85C0, 0x8E1570-0x8E1741, 0x8EB730-0x8EB868

; orbis_create_fence at 0x8d85c0
008D85C0  push    rbp
008D85C1  mov     rbp, rsp
008D85C4  push    rbx
008D85C5  push    rax
008D85C6  mov     edi, 18h
008D85CB  call    sub_37BF40
008D85D0  mov     rbx, rax
008D85D3  mov     rdi, rbx
008D85D6  call    orbis_fence_construct
008D85DB  mov     rax, rbx
008D85DE  add     rsp, 8
008D85E2  pop     rbx
008D85E3  pop     rbp
008D85E4  retn
008D85E5  align 10h

; orbis_fence_construct at 0x8e1570
008E1570  push    rbp
008E1571  mov     rbp, rsp
008E1574  push    rbx
008E1575  push    rax
008E1576  lea     rax, unk_195F6D8
008E157D  mov     rbx, rdi
008E1580  lea     rsi, aPs4fence; "PS4Fence"
008E1587  mov     edi, 4
008E158C  mov     edx, 4
008E1591  add     rax, 10h
008E1595  mov     [rbx], rax
008E1598  mov     qword ptr [rbx+8], 0
008E15A0  mov     dword ptr [rbx+10h], 0
008E15A7  call    sub_37AE70
008E15AC  mov     [rbx+8], rax
008E15B0  mov     dword ptr [rax], 0
008E15B6  add     rsp, 8
008E15BA  pop     rbx
008E15BB  pop     rbp
008E15BC  retn

; orbis_fence_destruct at 0x8e15f0
008E15F0  push    rbp
008E15F1  mov     rbp, rsp
008E15F4  push    rbx
008E15F5  push    rax
008E15F6  lea     rax, unk_195F6D8
008E15FD  lea     rcx, g_orbis_render_system
008E1604  mov     rbx, rdi
008E1607  add     rax, 10h
008E160B  mov     [rbx], rax
008E160E  mov     rdi, [rcx]
008E1611  mov     rsi, [rbx+8]
008E1615  test    rdi, rdi
008E1618  jz      short loc_8E1621
008E161A  call    orbis_defer_allocation_release
008E161F  jmp     short loc_8E1629
008E1621  mov     rdi, rsi
008E1624  call    sub_37B800
008E1629  mov     qword ptr [rbx+8], 0
008E1631  add     rsp, 8
008E1635  pop     rbx
008E1636  pop     rbp
008E1637  retn

; orbis_fence_base_destruct at 0x8e1640
008E1640  push    rbp
008E1641  mov     rbp, rsp
008E1644  push    rbx
008E1645  push    rax
008E1646  lea     rax, g_orbis_render_system
008E164D  mov     rbx, rdi
008E1650  mov     rsi, [rbx+8]
008E1654  mov     rdi, [rax]
008E1657  test    rdi, rdi
008E165A  jz      short loc_8E1663
008E165C  call    orbis_defer_allocation_release
008E1661  jmp     short loc_8E166B
008E1663  mov     rdi, rsi
008E1666  call    sub_37B800
008E166B  mov     qword ptr [rbx+8], 0
008E1673  add     rsp, 8
008E1677  pop     rbx
008E1678  pop     rbp
008E1679  retn

; orbis_fence_delete at 0x8e1680
008E1680  push    rbp
008E1681  mov     rbp, rsp
008E1684  push    rbx
008E1685  push    rax
008E1686  lea     rax, unk_195F6D8
008E168D  lea     rcx, g_orbis_render_system
008E1694  mov     rbx, rdi
008E1697  add     rax, 10h
008E169B  mov     [rbx], rax
008E169E  mov     rdi, [rcx]
008E16A1  mov     rsi, [rbx+8]
008E16A5  test    rdi, rdi
008E16A8  jz      short loc_8E16B1
008E16AA  call    orbis_defer_allocation_release
008E16AF  jmp     short loc_8E16B9
008E16B1  mov     rdi, rsi
008E16B4  call    sub_37B800
008E16B9  mov     rdi, rbx
008E16BC  add     rsp, 8
008E16C0  pop     rbx
008E16C1  pop     rbp
008E16C2  jmp     sub_37BF50

; orbis_fence_next_value at 0x8e16d0
008E16D0  push    rbp; Advances the fence sequence; on uint32 wrap it replaces the GPU label allocation before returning value one.
008E16D1  mov     rbp, rsp
008E16D4  push    rbx
008E16D5  push    rax
008E16D6  mov     rbx, rdi
008E16D9  mov     eax, [rbx+10h]
008E16DC  cmp     eax, 0FFFFFFFFh
008E16DF  jnz     short loc_8E1735
008E16E1  lea     rax, g_orbis_render_system
008E16E8  mov     dword ptr [rbx+10h], 0
008E16EF  mov     rsi, [rbx+8]
008E16F3  mov     rdi, [rax]
008E16F6  test    rdi, rdi
008E16F9  jz      short loc_8E1702
008E16FB  call    orbis_defer_allocation_release
008E1700  jmp     short loc_8E170A
008E1702  mov     rdi, rsi
008E1705  call    sub_37B800
008E170A  lea     rsi, aPs4fence; "PS4Fence"
008E1711  mov     edi, 4
008E1716  mov     edx, 4
008E171B  mov     qword ptr [rbx+8], 0
008E1723  call    sub_37AE70
008E1728  mov     [rbx+8], rax
008E172C  mov     dword ptr [rax], 0
008E1732  mov     eax, [rbx+10h]
008E1735  inc     eax
008E1737  mov     [rbx+10h], eax
008E173A  add     rsp, 8
008E173E  pop     rbx
008E173F  pop     rbp
008E1740  retn

; orbis_render_context_signal_fence at 0x8eb730
008EB730  push    rbp; Advances the fence value and emits a graphics or compute release-memory write to its GPU label.
008EB731  mov     rbp, rsp
008EB734  push    r15
008EB736  push    r14
008EB738  push    r12
008EB73A  push    rbx
008EB73B  mov     rbx, rdi
008EB73E  mov     eax, [rbx+4A24h]
008EB744  cmp     eax, 1
008EB747  jz      short loc_8EB791
008EB749  test    eax, eax
008EB74B  jnz     loc_8EB7E1
008EB751  imul    r15, [rbx+40D90h], 0E888h
008EB75C  mov     r14, [rsi+8]
008EB760  mov     rdi, rsi
008EB763  call    orbis_fence_next_value
008EB768  mov     r9d, eax
008EB76B  lea     rdi, [rbx+r15+5728h]
008EB773  mov     esi, 28h ; '('
008EB778  mov     edx, 0
008EB77D  mov     r8d, 1
008EB783  mov     rcx, r14
008EB786  push    0
008EB788  push    0
008EB78A  call    gnm_draw_command_buffer_write_release_mem_event
008EB78F  jmp     short loc_8EB7DD
008EB791  imul    r15, [rbx+4A28h], 1AE0h
008EB79C  imul    r12, [rbx+40D90h], 0F1E0h
008EB7A7  mov     r14, [rsi+8]
008EB7AB  mov     rdi, rsi
008EB7AE  call    orbis_fence_next_value
008EB7B3  add     r12, rbx
008EB7B6  mov     r9d, eax
008EB7B9  mov     esi, 2Fh ; '/'
008EB7BE  mov     edx, 0
008EB7C3  mov     r8d, 1
008EB7C9  mov     rcx, r14
008EB7CC  lea     rdi, [r15+r12+229E0h]
008EB7D4  push    0
008EB7D6  push    0
008EB7D8  call    gnm_compute_command_buffer_release_memory
008EB7DD  add     rsp, 10h
008EB7E1  pop     rbx
008EB7E2  pop     r12
008EB7E4  pop     r14
008EB7E6  pop     r15
008EB7E8  pop     rbp
008EB7E9  retn

; orbis_render_context_wait_fence at 0x8eb7f0
008EB7F0  mov     eax, [rdi+4A24h]; Emits a graphics or compute wait-on-address packet for the fence's current value.
008EB7F6  cmp     eax, 1
008EB7F9  jz      short loc_8EB82C
008EB7FB  test    eax, eax
008EB7FD  jnz     short locret_8EB867
008EB7FF  imul    rcx, [rdi+40D90h], 0E888h
008EB80A  mov     rax, [rsi+8]
008EB80E  mov     r8d, [rsi+10h]
008EB812  mov     edx, 0FFFFFFFFh
008EB817  mov     rsi, rax
008EB81A  lea     rdi, [rdi+rcx+5728h]
008EB822  mov     ecx, 5
008EB827  jmp     gnm_draw_command_buffer_wait_on_address
008EB82C  imul    rcx, [rdi+40D90h], 0F1E0h
008EB837  imul    rdx, [rdi+4A28h], 1AE0h
008EB842  mov     rax, [rsi+8]
008EB846  mov     r8d, [rsi+10h]
008EB84A  mov     rsi, rax
008EB84D  add     rcx, rdi
008EB850  lea     rdi, [rdx+rcx+229E0h]
008EB858  mov     edx, 0FFFFFFFFh
008EB85D  mov     ecx, 5
008EB862  jmp     gnm_compute_command_buffer_wait_on_address
008EB867  retn
