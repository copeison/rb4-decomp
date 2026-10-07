0000000000000920: push    rbp
0000000000000921: mov     rbp, rsp
0000000000000924: push    r15
0000000000000926: push    r14
0000000000000928: push    rbx
0000000000000929: push    rax
000000000000092A: mov     r14d, [rdi]
000000000000092D: mov     rbx, rsi
0000000000000930: lea     r15, [rdi+8]
0000000000000934: call    sub_1243220
0000000000000939: mov     rdi, rbx
000000000000093C: call    sub_1243230
0000000000000941: lea     rdi, runtime_run_finalizers; Runs the executable finalizer table once. Registered from start before game_main. Name is inferred from control flow.
0000000000000948: call    sub_1243230
000000000000094D: call    runtime_run_initializers; Runs the executable startup initializer table in reverse address order. Inferred from entry-point position and table walk.
0000000000000952: xor     edx, edx; envp
0000000000000954: mov     edi, r14d; argc
0000000000000957: mov     rsi, r15; argv
000000000000095A: call    game_main; Top-level game routine: initialize once, then run frames until requested shutdown. Receives argc/argv/envp from start but does not use them in this build.
000000000000095F: mov     ebx, eax
0000000000000961: mov     edi, ebx
0000000000000963: call    sub_1243240
0000000000000968: mov     edi, ebx
000000000000096A: call    sub_1243250
000000000000096F: ud2
