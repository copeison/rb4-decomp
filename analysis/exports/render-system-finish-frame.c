// Submits or abandons the active frame, clears transient objects, releases the frame lock, and polls default resources.
__int64 __fastcall render_system_finish_frame(__int64 a1, int a2, __m128 a3)
{
  __int64 v4; // r15
  __int64 v6; // r12
  __int64 v7; // rbx
  __int64 v8; // rdi
  unsigned __int64 v9; // rcx
  _BYTE *v11; // rdx
  unsigned __int64 v14; // r13
  __int64 v15; // rbx
  unsigned __int64 v16; // rax
  __int64 v17; // rdx
  __int64 v18; // rsi
  __int64 v19; // rdx
  unsigned __int64 v20; // r12
  _BYTE *v23; // r14
  __int64 v24; // r15
  __int64 v25; // rax
  unsigned __int64 v26; // rdx
  __int64 v27; // rsi
  int v28; // eax
  _QWORD *v29; // rbx
  __int64 v31; // [rsp+8h] [rbp-1D8h]
  int v32; // [rsp+14h] [rbp-1CCh]
  _BYTE *v33; // [rsp+18h] [rbp-1C8h]
  __int128 v34; // [rsp+20h] [rbp-1C0h]
  _BYTE v35[384]; // [rsp+30h] [rbp-1B0h] BYREF
  __int64 v36; // [rsp+1B0h] [rbp-30h]

  v4 = a1;
  v6 = a1 + 3584;
  v36 = 0x6365786562696C2FLL;
  v7 = g_render_system;
  if ( *(_BYTE *)(g_render_system + 64) != 0 )
  {
    sub_6BC3B0(*(_QWORD *)(g_render_system + 56), *(_DWORD *)(g_render_system + 68), a3);
    *(_BYTE *)(v7 + 64) = 0;
    *(_DWORD *)(v7 + 68) = 0;
  }
  sub_62B5B0(v6, *(_QWORD *)(v7 + 56), *(_QWORD *)(v4 + 280));
  if ( (_BYTE)a2 != 0 )
  {
    (*(void (__fastcall **)(__int64, __int64, __int64))(*(_QWORD *)v4 + 56LL))(v4, v4 + 184, 1);
    ++*(_QWORD *)(v4 + 168);
  }
  else
  {
    sub_62B960(v6);
    _RAX = 12;
    v11 = v35;
    __asm { vmovq   xmm0, rax }
    v33 = v35;
    __asm
    {
      vpslldq xmm0, xmm0, 8
      vmovdqu [rbp+var_1C0], xmm0
    }
    if ( *(_QWORD *)(v4 + 192) != 0 )
    {
      v32 = a2;
      v14 = 0;
      v31 = v4;
      do
      {
        v15 = *(_QWORD *)(*(_QWORD *)(v4 + 184) + 8 * v14);
        v16 = sub_448730(v15);
        v9 = HIDWORD(v16);
        if ( HIDWORD(v16) != 0 && (_DWORD)v16 != 0 )
        {
          (*(void (__fastcall **)(__int64, __int64, __int64, unsigned __int64))(*(_QWORD *)v15 + 24LL))(
            v15,
            v18,
            v17,
            v9);
          if ( v19 != 0 )
          {
            v20 = 0;
            do
            {
              _RAX = v33;
              __asm { vxorps  ymm0, ymm0, ymm0 }
              _RCX = 32 * v34;
              *(_QWORD *)&v34 = v34 + 1;
              __asm { vmovups ymmword ptr [rax+rcx], ymm0 }
              v23 = v33;
              v24 = 32 * v34;
              *(_QWORD *)&v33[32 * v34 - 32] = 0;
              v25 = *(_QWORD *)((*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v15 + 24LL))(v15) + 8 * v20++);
              *(_QWORD *)&v23[v24 - 24] = *(_QWORD *)(v25 + 368);
              *(_QWORD *)&v23[v24 - 16] = -1;
              *(_QWORD *)&v23[v24 - 8] = 4;
              (*(void (__fastcall **)(__int64))(*(_QWORD *)v15 + 24LL))(v15);
            }
            while ( v20 < v26 );
          }
        }
        v4 = v31;
        ++v14;
      }
      while ( v14 < *(_QWORD *)(v31 + 192) );
      v11 = v33;
      v27 = v34;
      LOBYTE(a2) = v32;
    }
    else
    {
      v27 = 0;
    }
    (*(void (__fastcall **)(_QWORD, __int64, _BYTE *, unsigned __int64))(**(_QWORD **)(v4 + 56) + 168LL))(
      *(_QWORD *)(v4 + 56),
      v27,
      v11,
      v9);
    (*(void (__fastcall **)(__int64, __int64, _QWORD))(*(_QWORD *)v4 + 56LL))(v4, v4 + 184, 0);
    if ( (*(unsigned int (__fastcall **)(__int64))(*(_QWORD *)v4 + 104LL))(v4) == 1 )
    {
      sub_249D40(&unk_19E7CD0, 0);
      if ( *(_BYTE *)(*(_QWORD *)(v4 + 296) + 180LL) != 0 )
        unk_19E7E60 = *(_QWORD *)(v4 + 160) & 1LL;
    }
    *(_QWORD *)(v4 + 192) = 0;
    ++*(_QWORD *)(v4 + 160);
    scePthreadMutexLock(v4 + 3752);
    v28 = *(_DWORD *)(v4 + 3744) + 1;
    *(_DWORD *)(v4 + 3744) = v28;
    v29 = *(_QWORD **)(v4 + 3760);
    if ( v29 != *(_QWORD **)(v4 + 3768) )
    {
      do
        sub_4F7270(*v29++);
      while ( v29 != *(_QWORD **)(v4 + 3768) );
      v29 = *(_QWORD **)(v4 + 3760);
      v28 = *(_DWORD *)(v4 + 3744);
    }
    *(_QWORD *)(v4 + 3768) = v29;
    *(_DWORD *)(v4 + 3744) = v28 - 1;
    scePthreadMutexUnlock(v4 + 3752);
  }
  *(_BYTE *)(v4 + 176) = 0;
  *(_BYTE *)(*(_QWORD *)(v4 + 56) + 8LL) = 0;
  scePthreadSelf(v8);
  *(_QWORD *)(v4 + 24) = 0;
  --*(_DWORD *)(v4 + 8);
  scePthreadMutexUnlock(v4 + 16);
  if ( (_BYTE)a2 == 0 )
    render_poll_default_resources((__int64 *)(v4 + 1976));
  return 0x6365786562696C2FLL;
}
