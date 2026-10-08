// Acquires and initializes four built-in GPU buffer bindings.
__int64 __fastcall render_system_initialize_builtin_buffers(__int64 *a1)
{
  _BYTE *v2; // rax
  __int64 v3; // rdx
  __int64 v4; // rcx
  _BYTE *v5; // r14
  _BYTE *v6; // rax
  __int64 v11; // rcx
  __int64 v15; // rcx
  __int64 v19; // rcx
  _BYTE *v23; // r14
  _BYTE *v24; // r14
  __int64 v25; // r15
  int v27; // eax
  _BYTE *v34; // r14
  _BYTE *v35; // rax
  __int64 result; // rax
  _BYTE *v37; // rbx

  v2 = sub_639F30(a1[339], 1u, -1);
  a1[464] = (__int64)v2;
  v3 = *((_QWORD *)v2 + 6);
  v4 = 16 * a1[340];
  *(_DWORD *)(v3 + v4) = 0;
  *(_DWORD *)(v3 + v4 + 4) = 0;
  v2[56] = 1;
  v5 = (_BYTE *)a1[464];
  if ( v5[56] != 0 )
  {
    (*(void (__fastcall **)(__int64))(*(_QWORD *)v5 + 16LL))(a1[464]);
    v5[56] = 0;
  }
  v6 = sub_639F30(a1[348], 1u, -1);
  _RSI = &unk_1B5D268;
  a1[465] = (__int64)v6;
  _RDX = *((_QWORD *)v6 + 6);
  __asm { vmovups xmm0, xmmword ptr [rsi] }
  _RCX = 16 * a1[349];
  __asm { vmovups xmmword ptr [rdx+rcx], xmm0 }
  v6[56] = 1;
  v11 = a1[465];
  __asm { vmovups xmm0, xmmword ptr [rsi] }
  _RDX = *(_QWORD *)(v11 + 48);
  _RAX = 16 * a1[349];
  __asm { vmovups xmmword ptr [rdx+rax+10h], xmm0 }
  *(_BYTE *)(v11 + 56) = 1;
  v15 = a1[465];
  __asm { vmovups xmm0, xmmword ptr [rsi] }
  _RDX = *(_QWORD *)(v15 + 48);
  _RAX = 16 * a1[349];
  __asm { vmovups xmmword ptr [rdx+rax+20h], xmm0 }
  *(_BYTE *)(v15 + 56) = 1;
  v19 = a1[465];
  __asm { vmovups xmm0, xmmword ptr [rsi] }
  _RDX = *(_QWORD *)(v19 + 48);
  _RAX = 16 * a1[349];
  __asm { vmovups xmmword ptr [rdx+rax+30h], xmm0 }
  *(_BYTE *)(v19 + 56) = 1;
  v23 = (_BYTE *)a1[465];
  if ( v23[56] != 0 )
  {
    (*(void (__fastcall **)(__int64))(*(_QWORD *)v23 + 16LL))(a1[465]);
    v23[56] = 0;
  }
  v24 = sub_639F30(a1[352], 1u, -1);
  a1[466] = (__int64)v24;
  *(_DWORD *)(*((_QWORD *)v24 + 6) + 16 * a1[353]) = -1082130432;
  v24[56] = 1;
  v25 = a1[354];
  if ( byte_1A712C8[0] == 0 )
  {
    *(double *)&_XMM0 = _cxa_guard_acquire(byte_1A712C8);
    if ( v27 != 0 )
    {
      _RAX = &unk_1A712B8;
      __asm
      {
        vxorps  xmm0, xmm0, xmm0
        vmovups xmmword ptr [rax], xmm0
      }
      _cxa_guard_release(byte_1A712C8);
    }
  }
  _RCX = &unk_1A712B8;
  _RAX = *((_QWORD *)v24 + 6);
  _R15 = 16 * v25;
  __asm
  {
    vmovups xmm0, xmmword ptr [rcx]
    vmovups xmmword ptr [rax+r15], xmm0
  }
  v24[56] = 1;
  v34 = (_BYTE *)a1[466];
  if ( v34[56] != 0 )
  {
    (*(void (__fastcall **)(__int64))(*(_QWORD *)v34 + 16LL))(a1[466]);
    v34[56] = 0;
  }
  v35 = sub_639F30(a1[355], 1u, -1);
  a1[467] = (__int64)v35;
  result = sub_5F7E70((__int64)v35);
  v37 = (_BYTE *)a1[467];
  if ( v37[56] != 0 )
  {
    result = (*(__int64 (__fastcall **)(_BYTE *))(*(_QWORD *)v37 + 16LL))(v37);
    v37[56] = 0;
  }
  return result;
}
