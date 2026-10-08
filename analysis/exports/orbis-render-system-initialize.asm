00000000008D7B20: push    rbp
00000000008D7B21: mov     rbp, rsp
00000000008D7B24: push    r15
00000000008D7B26: push    r14
00000000008D7B28: push    r13
00000000008D7B2A: push    r12
00000000008D7B2C: push    rbx
00000000008D7B2D: sub     rsp, 18h
00000000008D7B31: mov     r13, cs:qword_19A9B88
00000000008D7B38: mov     rbx, rdi
00000000008D7B3B: mov     edi, 0FFh
00000000008D7B40: xor     esi, esi
00000000008D7B42: xor     edx, edx
00000000008D7B44: xor     ecx, ecx
00000000008D7B46: mov     rax, [r13+0]
00000000008D7B4A: mov     [rbp-30h], rax
00000000008D7B4E: call    sceVideoOutOpen; PS4 SDK 5.008 import resolved from NID Up36PTk687E; stub: target/lib/libSceVideoOut_stub_weak.a
00000000008D7B53: xor     esi, esi
00000000008D7B55: mov     edi, eax
00000000008D7B57: mov     [rbx+0EDCh], eax
00000000008D7B5D: call    sceVideoOutSetFlipRate; PS4 SDK 5.008 import resolved from NID CBiu4mCE1DA; stub: target/lib/libSceVideoOut_stub_weak.a
00000000008D7B62: mov     edi, [rbx+0EDCh]
00000000008D7B68: mov     esi, 438h
00000000008D7B6D: xor     edx, edx
00000000008D7B6F: call    sceVideoOutSetWindowModeMargins; PS4 SDK 5.008 import resolved from NID MTxxrOCeSig; stub: target/lib/libSceVideoOut_stub_weak.a
00000000008D7B74: lea     rdi, [rbx+0EE0h]
00000000008D7B7B: lea     rsi, aEopQueue; "EOP QUEUE"
00000000008D7B82: call    sceKernelCreateEqueue; PS4 SDK 5.008 import resolved from NID D0OdFMjp46I; stub: target/lib/libkernel_stub_weak.a
00000000008D7B87: mov     rdi, [rbx+0EE0h]
00000000008D7B8E: mov     esi, 40h ; '@'
00000000008D7B93: xor     edx, edx
00000000008D7B95: call    j_sceGnmAddEqEvent
00000000008D7B9A: mov     rdi, [rbx+0EE0h]
00000000008D7BA1: mov     esi, [rbx+0EDCh]
00000000008D7BA7: xor     edx, edx
00000000008D7BA9: call    sceVideoOutAddFlipEvent; PS4 SDK 5.008 import resolved from NID HXzjK9yI30k; stub: target/lib/libSceVideoOut_stub_weak.a
00000000008D7BAE: mov     rdi, rbx
00000000008D7BB1: call    orbis_create_default_vertex_buffer
00000000008D7BB6: mov     rdi, rbx
00000000008D7BB9: call    orbis_create_identity_instance_buffer
00000000008D7BBE: mov     edi, 8
00000000008D7BC3: call    sub_37BF40
00000000008D7BC8: lea     rcx, unk_195EDF8
00000000008D7BCF: mov     rdi, rbx
00000000008D7BD2: mov     rsi, rax
00000000008D7BD5: add     rcx, 10h
00000000008D7BD9: mov     [rax], rcx
00000000008D7BDC: call    sub_3DEDB0
00000000008D7BE1: mov     edi, 28h ; '('
00000000008D7BE6: call    sub_37BF40
00000000008D7BEB: mov     r14, rax
00000000008D7BEE: mov     rdi, r14
00000000008D7BF1: call    sub_8E24A0
00000000008D7BF6: mov     rdi, rbx
00000000008D7BF9: mov     rsi, r14
00000000008D7BFC: call    sub_3DED70
00000000008D7C01: mov     edi, offset loc_44890
00000000008D7C06: call    sub_37BF40
00000000008D7C0B: mov     r14, rax
00000000008D7C0E: mov     rdi, r14
00000000008D7C11: call    sub_8E72B0
00000000008D7C16: mov     rdi, rbx
00000000008D7C19: mov     rsi, r14
00000000008D7C1C: call    sub_3DEDC0
00000000008D7C21: lea     r12, [rbp-38h]
00000000008D7C25: lea     r14, [rbx+10C0h]
00000000008D7C2C: mov     rdi, r12
00000000008D7C2F: mov     [rbx+0EF0h], r14
00000000008D7C36: call    scePthreadCondattrInit; PS4 SDK 5.008 import resolved from NID m5-2bsNfv7s; stub: target/lib/libkernel_stub_weak.a
00000000008D7C3B: lea     r15, [rbx+0EF8h]
00000000008D7C42: lea     rdx, aCondition_3; "Condition"
00000000008D7C49: mov     rsi, r12
00000000008D7C4C: mov     rdi, r15
00000000008D7C4F: call    scePthreadCondInit; PS4 SDK 5.008 import resolved from NID 2Tb92quprl0; stub: target/lib/libkernel_stub_weak.a
00000000008D7C54: lea     rsi, orbis_submit_done_thread_entry
00000000008D7C5B: lea     rdi, [rbx+1030h]
00000000008D7C62: lea     rcx, aSubmitdonethre; "SubmitDoneThread"
00000000008D7C69: mov     r8, 0FFFFFFFFFFFFFFFFh
00000000008D7C70: mov     r9d, 2BBh
00000000008D7C76: mov     rdx, rbx
00000000008D7C79: push    0
00000000008D7C7B: push    0
00000000008D7C7D: call    sub_259210
00000000008D7C82: add     rsp, 10h
00000000008D7C86: lea     rdi, [rbx+1038h]
00000000008D7C8D: mov     byte ptr [rbx+0F08h], 1
00000000008D7C94: mov     qword ptr [rbx+0F00h], 0
00000000008D7C9F: call    sub_25C430
00000000008D7CA4: mov     rdi, r14
00000000008D7CA7: call    scePthreadMutexLock; PS4 SDK 5.008 import resolved from NID 9UK1vLZQft4; stub: target/lib/libkernel_stub_weak.a
00000000008D7CAC: mov     eax, [rbx+10B8h]
00000000008D7CB2: inc     eax
00000000008D7CB4: mov     [rbx+10B8h], eax
00000000008D7CBA: cmp     qword ptr [rbx+0F00h], 0
00000000008D7CC2: jnz     short loc_8D7CEF
00000000008D7CC4: nop     word ptr [rax+rax+00000000h]
00000000008D7CD0: mov     rsi, [rbx+0EF0h]
00000000008D7CD7: mov     rdi, r15
00000000008D7CDA: call    scePthreadCondWait; PS4 SDK 5.008 import resolved from NID WKAXJ4XBPQ4; stub: target/lib/libkernel_stub_weak.a
00000000008D7CDF: cmp     qword ptr [rbx+0F00h], 0
00000000008D7CE7: jz      short loc_8D7CD0
00000000008D7CE9: mov     eax, [rbx+10B8h]
00000000008D7CEF: dec     eax
00000000008D7CF1: mov     rdi, r14
00000000008D7CF4: mov     [rbx+10B8h], eax
00000000008D7CFA: call    scePthreadMutexUnlock; PS4 SDK 5.008 import resolved from NID tn3VlD0hG60; stub: target/lib/libkernel_stub_weak.a
00000000008D7CFF: call    sceSystemServiceHideSplashScreen; PS4 SDK 5.008 import resolved from NID Vo5V8KAwCmk; stub: target/lib/libSceSystemService_stub.a;target/lib/libSceSystemService_stub_weak.a
00000000008D7D04: mov     rax, [r13+0]
00000000008D7D08: cmp     rax, [rbp-30h]
00000000008D7D0C: jnz     short loc_8D7D1D
00000000008D7D0E: add     rsp, 18h
00000000008D7D12: pop     rbx
00000000008D7D13: pop     r12
00000000008D7D15: pop     r13
00000000008D7D17: pop     r14
00000000008D7D19: pop     r15
00000000008D7D1B: pop     rbp
00000000008D7D1C: retn
00000000008D7D1D: call    __stack_chk_fail; PS4 SDK 5.008 import resolved from NID Ou3iL1abvng; stub: target/lib/libkernel_stub_weak.a
