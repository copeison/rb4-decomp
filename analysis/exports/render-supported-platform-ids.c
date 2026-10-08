// Reads platform_mgr.supported_platforms into a vector of platform identifiers.
__int64 *__fastcall render_supported_platform_ids(__int64 *a1)
{
  int v2; // eax
  int v3; // eax
  __int64 v4; // rsi
  __int64 v5; // rax
  __int64 v8; // r13
  __int16 v9; // ax
  unsigned __int64 v10; // r14
  __int64 *v11; // rbx
  __int64 v12; // r15
  __int64 v13; // r12
  __int64 v14; // r12
  unsigned __int64 v15; // rax
  char *v16; // r14
  unsigned __int64 v17; // r12
  int v18; // eax
  _DWORD *v19; // rbx
  __int64 v20; // rsi
  __int64 v21; // r13
  __int64 v22; // rax
  char *v23; // rbx
  __int64 v24; // rdx
  __int64 v25; // rcx
  __int64 v27; // [rsp+8h] [rbp-68h]
  __int64 v28; // [rsp+10h] [rbp-60h]
  __int64 v29; // [rsp+18h] [rbp-58h]
  int v30; // [rsp+20h] [rbp-50h]
  __int64 *v31; // [rsp+28h] [rbp-48h]
  __int64 v32; // [rsp+30h] [rbp-40h] BYREF
  _QWORD v33[7]; // [rsp+38h] [rbp-38h] BYREF

  _R15 = a1;
  v33[1] = 0x6365786562696C2FLL;
  if ( byte_19FE238 == 0 )
  {
    _cxa_guard_acquire(&byte_19FE238);
    if ( v2 != 0 )
    {
      qword_19FE230 = 19246190;
      _cxa_guard_release(&byte_19FE238);
    }
  }
  if ( qword_19FE230 == 19246190 )
  {
    sub_256FD0(v33, "platform_mgr");
    qword_19FE230 = v33[0];
  }
  if ( byte_19FE248 == 0 )
  {
    _cxa_guard_acquire(&byte_19FE248);
    if ( v3 != 0 )
    {
      qword_19FE240 = 19246190;
      _cxa_guard_release(&byte_19FE248);
    }
  }
  v4 = qword_19FE240;
  if ( qword_19FE240 == 19246190 )
  {
    sub_256FD0(&v32, "supported_platforms");
    v4 = v32;
    qword_19FE240 = v32;
  }
  v5 = sub_368CD0(qword_19FE230, v4);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  v8 = v5;
  __asm { vmovups xmmword ptr [r15], xmm0 }
  _R15[2] = 0;
  v31 = _R15 + 3;
  nullsub_18(_R15 + 3, "EASTL vector");
  v9 = *(_WORD *)(v8 + 20);
  v10 = (unsigned int)(v9 - 1);
  if ( (_R15[2] - *_R15) >> 2 < v10 )
  {
    v11 = _R15;
    v12 = sub_252CF0(v31, 4 * v10, 0);
    v13 = v11[1] - *v11;
    memmove(v12, *v11, v13);
    v14 = v12 + v13;
    if ( *v11 != 0 )
      sub_252D30(v31, *v11, v11[2] - *v11);
    v15 = v12 + 4 * v10;
    *v11 = v12;
    v11[1] = v14;
    _R15 = v11;
    v11[2] = v15;
    v9 = *(_WORD *)(v8 + 20);
  }
  if ( (v9 & 0xFFFFFFFE) != 0 )
  {
    v16 = "so.1";
    v17 = 1;
    v28 = v8;
    do
    {
      v18 = sub_EB40(&v16[*(_QWORD *)v8], v8);
      v19 = (_DWORD *)_R15[1];
      if ( (unsigned __int64)v19 >= _R15[2] )
      {
        v20 = *_R15;
        v30 = v18;
        v21 = ((__int64)v19 - *_R15) >> 1;
        if ( v19 == (_DWORD *)*_R15 )
          v21 = 1;
        if ( v21 != 0 )
        {
          v22 = sub_252CF0(v31, 4 * v21, 0);
          v20 = *_R15;
          v19 = (_DWORD *)_R15[1];
          v29 = v22;
        }
        else
        {
          v22 = 0;
          v29 = 0;
        }
        v23 = (char *)v19 - v20;
        v27 = v22;
        memmove(v22, v20, v23);
        *(_DWORD *)&v23[v29] = v30;
        v24 = (__int64)&v23[v29 + 4];
        if ( *_R15 != 0 )
        {
          sub_252D30(v31, *_R15, _R15[2] - *_R15);
          v24 = (__int64)&v23[v29 + 4];
        }
        v25 = v29 + 4 * v21;
        v8 = v28;
        *_R15 = v27;
        _R15[1] = v24;
        _R15[2] = v25;
      }
      else
      {
        _R15[1] = (__int64)(v19 + 1);
        *v19 = v18;
      }
      ++v17;
      v16 += 16;
    }
    while ( v17 < (unsigned int)*(__int16 *)(v8 + 20) );
  }
  return _R15;
}
