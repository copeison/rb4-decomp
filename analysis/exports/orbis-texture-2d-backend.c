/* Generated Hex-Rays evidence for the Orbis 2D texture backend. */
/* The initializer at 0x8D6460 does not currently decompile; see the paired .asm export. */

/* 0x8D6310 */
__int64 __fastcall orbis_texture_2d_destruct(_QWORD *a1, __int64 a2)
{
  __int64 v3; // rdi
  __int64 v4; // rdi
  __int64 v5; // rdi
  __int64 v6; // rdi
  __int64 v7; // rdi
  __int64 v8; // rdi
  __int64 v9; // rdi
  __int64 v10; // r14

  *a1 = &unk_195EC80;
  v3 = a1[61];
  if ( v3 != 0 )
    sub_37BF50(v3);
  a1[61] = 0;
  v4 = a1[62];
  if ( v4 != 0 )
    sub_37BF50(v4);
  a1[62] = 0;
  v5 = a1[51];
  if ( v5 != 0 )
    sub_37BF50(v5);
  a1[51] = 0;
  v6 = a1[52];
  if ( v6 != 0 )
    sub_37BF50(v6);
  a1[52] = 0;
  v7 = a1[53];
  if ( v7 != 0 )
    sub_37BF50(v7);
  a1[53] = 0;
  v8 = a1[63];
  if ( v8 != 0 )
    sub_37BF50(v8);
  a1[63] = 0;
  v9 = a1[64];
  if ( v9 != 0 )
    sub_37BF70(v9, a2);
  v10 = a1[59];
  if ( v10 != 0 && (unsigned int)Atomic_fetch_sub_4(v10 + 8, 1, 5) == 1 )
  {
    (**(void (__fastcall ***)(__int64))v10)(v10);
    if ( (unsigned int)Atomic_fetch_sub_4(v10 + 12, 1, 5) == 1 )
      (*(void (__fastcall **)(__int64))(*(_QWORD *)v10 + 8LL))(v10);
  }
  return sub_6901D0(a1);
}

/* 0x8D6440 */
double __fastcall orbis_texture_2d_delete(_QWORD *a1, __int64 a2)
{
  __int64 v2; // rax
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v2;
  orbis_texture_2d_destruct(a1, a2);
  return sub_37BF50(a1);
}

/* 0x8D6D10 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_texture_2d_update_gpu_data(__int64 a1)
{
  __int64 v2; // r13
  unsigned __int64 v3; // rbx
  __int64 v4; // r14
  __int64 v6; // [rsp+0h] [rbp-70h] BYREF
  __int64 v7; // [rsp+8h] [rbp-68h] BYREF
  _QWORD v8[12]; // [rsp+10h] [rbp-60h] BYREF

  v2 = a1 + 312;
  v8[6] = 0x6365786562696C2FLL;
  *(_QWORD *)(a1 + 456) = (*(_DWORD *)(a1 + 456) & 1) == 0;
  if ( sub_6832D0(a1 + 312) != -1 )
  {
    v3 = 0;
    v4 = v2;
    do
    {
      sub_10FC210(v8, *(_QWORD *)(a1 + 8LL * *(_QWORD *)(a1 + 456) + 408), (unsigned int)v3, 0);
      sub_10FD200(&v7, &v6, *(_QWORD *)(a1 + 8LL * *(_QWORD *)(a1 + 456) + 408), (unsigned int)v3, 0);
      sub_10FDB10(
        v7
      + ((*(_QWORD *)(*(_QWORD *)(a1 + 464) + 8LL * *(_QWORD *)(a1 + 456)) + *(unsigned int *)(a1 + 436) - 1LL)
       & -(__int64)*(unsigned int *)(a1 + 436)),
        *(_QWORD *)(v4 + 24),
        v8);
      v4 = *(_QWORD *)(v4 + 40);
      ++v3;
    }
    while ( v3 < sub_6832D0(v2) + 1 );
  }
  return 0x6365786562696C2FLL;
}

/* 0x8D71E0 */
__int64 __fastcall orbis_texture_2d_render_target(__int64 a1)
{
  __int64 result; // rax

  result = *(_QWORD *)(a1 + 8LL * *(_QWORD *)(*(_QWORD *)(g_orbis_render_system + 112) + 32LL) + 488);
  if ( result == 0 )
    return *(_QWORD *)(a1 + 488);
  return result;
}

/* 0x8D7240 */
__int64 __fastcall orbis_texture_2d_depth_target(__int64 a1)
{
  return *(_QWORD *)(a1 + 504);
}
