; IDA disassembly evidence for the Orbis occlusion-query backend.
; Functions: 0x8E28F0-0x8E2A78

; orbis_occlusion_query_destruct at 0x8e28f0
008E28F0  jmp     occlusion_query_destruct

; orbis_occlusion_query_delete at 0x8e2900
008E2900  push    rbp
008E2901  mov     rbp, rsp
008E2904  push    rbx
008E2905  push    rax
008E2906  mov     rbx, rdi
008E2909  call    occlusion_query_destruct
008E290E  mov     rdi, rbx
008E2911  add     rsp, 8
008E2915  pop     rbx
008E2916  pop     rbp
008E2917  jmp     sub_37BF50

; orbis_occlusion_query_begin at 0x8e2920
008E2920  push    rbp
008E2921  mov     rbp, rsp
008E2924  push    r15
008E2926  push    r14
008E2928  push    rbx
008E2929  push    rax
008E292A  mov     rbx, rsi
008E292D  mov     r14, rdi
008E2930  imul    rax, [rbx+40D90h], 0E888h
008E293B  mov     rdx, [rbx+rax+5730h]
008E2943  lea     r15, [rbx+rax+5730h]
008E294B  mov     rcx, rdx
008E294E  sub     rcx, [rbx+rax+5738h]
008E2956  shr     rcx, 2
008E295A  cmp     ecx, 44h ; 'D'
008E295D  ja      short loc_8E2982
008E295F  mov     rdx, [rbx+rax+5748h]
008E2967  lea     rdi, [rbx+rax+5728h]
008E296F  mov     esi, 45h ; 'E'
008E2974  call    qword ptr [rbx+rax+5740h]
008E297B  test    al, al
008E297D  jz      short loc_8E2992
008E297F  mov     rdx, [r15]
008E2982  add     rdx, 0FFFFFFFFFFFFFF00h
008E2989  and     rdx, 0FFFFFFFFFFFFFFF0h
008E298D  mov     [r15], rdx
008E2990  jmp     short loc_8E2994
008E2992  xor     edx, edx
008E2994  mov     [r14+40h], rdx
008E2998  xor     esi, esi
008E299A  imul    rax, [rbx+40D90h], 0E888h
008E29A5  lea     rdi, [rbx+rax+5728h]
008E29AD  call    sub_10CAB40
008E29B2  imul    rax, [rbx+40D90h], 0E888h
008E29BD  mov     esi, 1
008E29C2  xor     edx, edx
008E29C4  lea     rdi, [rbx+rax+5728h]
008E29CC  add     rsp, 8
008E29D0  pop     rbx
008E29D1  pop     r14
008E29D3  pop     r15
008E29D5  pop     rbp
008E29D6  jmp     sub_10CA130

; orbis_occlusion_query_end at 0x8e29e0
008E29E0  push    rbp
008E29E1  mov     rbp, rsp
008E29E4  push    rbx
008E29E5  push    rax
008E29E6  mov     rbx, rsi
008E29E9  mov     rdx, [rdi+40h]
008E29ED  mov     esi, 1
008E29F2  imul    rax, [rbx+40D90h], 0E888h
008E29FD  lea     rdi, [rbx+rax+5728h]
008E2A05  call    sub_10CAB40
008E2A0A  imul    rax, [rbx+40D90h], 0E888h
008E2A15  xor     esi, esi
008E2A17  xor     edx, edx
008E2A19  lea     rdi, [rbx+rax+5728h]
008E2A21  add     rsp, 8
008E2A25  pop     rbx
008E2A26  pop     rbp
008E2A27  jmp     sub_10CA130

; orbis_occlusion_query_begin_conditional_render at 0x8e2a30
008E2A30  imul    rcx, [rsi+40D90h], 0E888h
008E2A3B  mov     rax, [rdi+40h]
008E2A3F  xor     edx, edx
008E2A41  lea     rdi, [rsi+rcx+5728h]
008E2A49  mov     ecx, 1
008E2A4E  mov     rsi, rax
008E2A51  jmp     sub_10CAA50

; orbis_occlusion_query_end_conditional_render at 0x8e2a60
008E2A60  imul    rax, [rsi+40D90h], 0E888h
008E2A6B  lea     rdi, [rsi+rax+5728h]
008E2A73  jmp     sub_10CAAE0
