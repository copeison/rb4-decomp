// Runs the Orbis EOP and flip-event loop, advances buffer fences, applies vsync state, and submits flips.
__int64 __fastcall orbis_submit_done_thread_run(__int64 a1)
{
  __int64 v1; // r14
  int v2; // eax
  unsigned __int64 v3; // rax
  unsigned int v4; // r13d
  unsigned int v5; // r12d
  __int64 v6; // r15
  int v7; // eax
  unsigned __int64 v8; // r14
  char v11; // cf
  __int64 v12; // rax
  __int64 v13; // rdx
  __int64 v14; // rdx
  __int64 v15; // rcx
  __int64 v16; // rax
  unsigned int v17; // eax
  __int64 v18; // rax
  __int64 v19; // rdx
  unsigned __int64 v20; // rcx
  __int64 v21; // rdx
  __int64 v22; // rsi
  __int64 v23; // rdx
  unsigned int v24; // eax
  __int64 v26; // [rsp+0h] [rbp-130h]
  __int64 v27; // [rsp+8h] [rbp-128h]
  __int64 v28; // [rsp+10h] [rbp-120h]
  unsigned __int64 v29; // [rsp+18h] [rbp-118h]
  unsigned __int64 v30; // [rsp+28h] [rbp-108h]
  __int64 v31; // [rsp+30h] [rbp-100h]
  char v32[24]; // [rsp+38h] [rbp-F8h] BYREF
  unsigned __int64 v33; // [rsp+50h] [rbp-E0h]
  int v34; // [rsp+78h] [rbp-B8h] BYREF
  int v35; // [rsp+7Ch] [rbp-B4h] BYREF
  _BYTE v36[128]; // [rsp+80h] [rbp-B0h] BYREF
  __int64 v37; // [rsp+100h] [rbp-30h]

  v1 = a1;
  v37 = 0x6365786562696C2FLL;
  sub_3DE080(a1);
  if ( *(_BYTE *)(v1 + 64) != 0 )
    sub_3DEF20(v1);
  v31 = *(_QWORD *)(v1 + 56);
  sub_3DE0B0(v1);
  scePthreadMutexLock(v1 + 4288);
  v2 = *(_DWORD *)(v1 + 4280);
  *(_QWORD *)(v1 + 3840) = 1;
  *(_DWORD *)(v1 + 4280) = v2;
  v28 = v1 + 4288;
  scePthreadMutexUnlock(v1 + 4288);
  v26 = v1 + 3832;
  scePthreadCondSignal(v1 + 3832);
  v3 = __rdtsc();
  if ( *(_BYTE *)(v1 + 3848) != 0 )
  {
    v4 = 2;
    v5 = 0;
    v27 = v1;
    v30 = v3;
    v29 = 0;
    do
    {
      while ( 1 )
      {
        v35 = 0;
        v34 = (int)&loc_F4240;
        if ( (unsigned int)sceKernelWaitEqueue(*(_QWORD *)(v1 + 3808), v36, 4, &v35, &v34) == 0 )
          break;
        scePthreadMutexLock(v28);
        ++*(_DWORD *)(v1 + 4280);
        v30 = __rdtsc();
        j_sceGnmSubmitDone();
        --*(_DWORD *)(v1 + 4280);
        scePthreadMutexUnlock(v28);
        v29 = 0;
        if ( *(_BYTE *)(v1 + 3848) == 0 )
          return 0;
      }
      if ( v35 != 0 )
      {
        v6 = 0;
        do
        {
          v7 = *(__int16 *)&v36[32 * v6 + 8];
          if ( v7 == -13 )
          {
            sceVideoOutGetFlipStatus(*(unsigned int *)(v1 + 3804), v32);
            if ( v33 <= 1 )
            {
              v18 = (*(__int64 (__fastcall **)(_QWORD))(**(_QWORD **)(v1 + 112) + 24LL))(*(_QWORD *)(v1 + 112));
              if ( v19 != 0 )
              {
                v20 = v33;
                v21 = 8 * v19;
                do
                {
                  v22 = *(_QWORD *)(*(_QWORD *)v18 + 368LL);
                  if ( v22 != 0 )
                    --*(_DWORD *)(*(_QWORD *)(v22 + 512) + 4 * v20);
                  v18 += 8;
                  v21 -= 8;
                }
                while ( v21 != 0 );
              }
            }
          }
          else if ( v7 == -14 )
          {
            scePthreadMutexLock(v28);
            ++*(_DWORD *)(v1 + 4280);
            if ( *(int *)((char *)&dword_22880[10 * v5] + v31) == 0
              && *(int *)((char *)&dword_22884[10 * v5] + v31) == 0
              && *(int *)((char *)&dword_22888[10 * v5] + v31) == 0
              && *(int *)((char *)&dword_2288C[10 * v5] + v31) == 0
              && *(int *)((char *)&dword_22890[10 * v5] + v31) == 0
              && *(int *)((char *)&dword_22894[10 * v5] + v31) == 0
              && *(int *)((char *)&dword_22898[10 * v5] + v31) == 0
              && *(int *)((char *)&dword_2289C[10 * v5] + v31) == 0
              && *(int *)((char *)&dword_228A0[10 * v5] + v31) == 0
              && *(int *)((char *)&dword_228A4[10 * v5] + v31) == 0 )
            {
              goto LABEL_23;
            }
            v8 = __rdtsc();
            v29 += ((unsigned int)v8 | ((unsigned __int64)HIDWORD(v8) << 32)) - v30;
            *(double *)&_XMM0 = performance_counter_ticks_to_milliseconds(v29);
            __asm
            {
              vcvtsd2ss xmm0, xmm0, xmm0
              vucomiss xmm0, cs:dword_12D13F4
            }
            v30 = v8;
            v1 = v27;
            if ( !v11 )
            {
LABEL_23:
              v30 = __rdtsc();
              j_sceGnmSubmitDone();
              v29 = 0;
            }
            v12 = (*(__int64 (__fastcall **)(_QWORD))(**(_QWORD **)(v1 + 112) + 24LL))(*(_QWORD *)(v1 + 112));
            if ( v13 != 0 )
            {
              v14 = 8 * v13;
              do
              {
                v15 = *(_QWORD *)(*(_QWORD *)v12 + 368LL);
                if ( v15 != 0 )
                  ++*(_DWORD *)(*(_QWORD *)(v15 + 512) + 4LL * v5);
                v12 += 8;
                v14 -= 8;
              }
              while ( v14 != 0 );
            }
            *(_QWORD *)(v1 + 3840) = 1;
            --*(_DWORD *)(v1 + 4280);
            scePthreadMutexUnlock(v28);
            scePthreadCondSignal(v26);
            v16 = *(_QWORD *)(v1 + 296);
            if ( *(_BYTE *)(v16 + 152) != 0 )
              v17 = *(_DWORD *)(v16 + 20);
            else
              v17 = 0;
            if ( *(_DWORD *)(v1 + 4344) != v17 )
            {
              *(_DWORD *)(v1 + 4344) = v17;
              sceVideoOutSetFlipRate(*(unsigned int *)(v1 + 3804), v17 == 2);
              v17 = *(_DWORD *)(v1 + 4344);
            }
            v23 = 6;
            if ( v17 <= 2 )
              v23 = *((unsigned int *)qword_12D1488 + (int)v17);
            sceVideoOutSubmitFlip(*(unsigned int *)(v1 + 3804), v5, v23, v4);
            v24 = v5 + 1;
            v4 = v5;
            if ( v5 == 1 )
              v24 = 0;
            v5 = v24;
          }
          ++v6;
        }
        while ( (_DWORD)v6 != v35 );
      }
    }
    while ( *(_BYTE *)(v1 + 3848) != 0 );
  }
  return 0;
}
