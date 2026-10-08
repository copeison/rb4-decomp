/* Generated Hex-Rays evidence for the Orbis cube texture-array backend. */

/* 0x8E6670 */
__int64 __fastcall orbis_texture_array_cube_destruct(_QWORD *a1)
{
  __int64 v1; // rax
  double v3; // xmm0_8
  __int64 v4; // rdi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195FC50;
  v3 = orbis_defer_allocation_release(g_orbis_render_system, a1[44]);
  v4 = a1[43];
  if ( v4 != 0 )
    v3 = sub_37BF50(v4);
  a1[43] = 0;
  return sub_69ACD0(a1, v3);
}

/* 0x8E66D0 */
double __fastcall orbis_texture_array_cube_delete(_QWORD *a1)
{
  __int64 v1; // rax
  double v3; // xmm0_8
  __int64 v4; // rdi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195FC50;
  v3 = orbis_defer_allocation_release(g_orbis_render_system, a1[44]);
  v4 = a1[43];
  if ( v4 != 0 )
    v3 = sub_37BF50(v4);
  a1[43] = 0;
  sub_69ACD0(a1, v3);
  return sub_37BF50(a1);
}

/* 0x8E6730 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_texture_array_cube_initialize_backend(__int64 a1)
{
  __int64 v2; // r14
  __int64 v3; // rbx
  unsigned int v4; // r13d
  unsigned int v5; // r15d
  unsigned int v6; // eax
  unsigned __int64 v9; // rax
  unsigned int v10; // r12d
  unsigned __int64 v11; // r15
  int v12; // eax
  __int64 v13; // rdi
  __int64 v14; // rsi
  __int64 v15; // r12
  __int64 v16; // rcx
  __int64 v17; // r15
  unsigned __int64 v18; // r13
  __int64 v19; // r14
  unsigned int v20; // r12d
  int v22; // [rsp+0h] [rbp-D0h]
  __int64 v23; // [rsp+8h] [rbp-C8h]
  __int64 v24; // [rsp+10h] [rbp-C0h]
  __int64 v25; // [rsp+18h] [rbp-B8h]
  __int64 v26; // [rsp+20h] [rbp-B0h]
  __int64 v27; // [rsp+28h] [rbp-A8h] BYREF
  __int64 v28; // [rsp+30h] [rbp-A0h] BYREF
  _BYTE v29[48]; // [rsp+38h] [rbp-98h] BYREF
  _DWORD v30[13]; // [rsp+68h] [rbp-68h] BYREF
  int v31; // [rsp+9Ch] [rbp-34h] BYREF
  __int64 v32; // [rsp+A0h] [rbp-30h]

  v2 = a1;
  v32 = 0x6365786562696C2FLL;
  v3 = *(_QWORD *)(a1 + 312);
  v4 = *(_DWORD *)(v3 + 20);
  v5 = sub_10DEF20();
  v6 = sub_8E1790(v4);
  sub_10F4A00(v5, &v31, 9, v6, 1);
  sub_10DC780(v30);
  v30[0] = 11;
  v30[1] = *(_DWORD *)(v2 + 112);
  v30[2] = *(_DWORD *)(v2 + 116);
  v30[3] = 1;
  v30[4] = 0;
  v30[6] = -286331153 * ((*(_QWORD *)(v2 + 320) - *(_QWORD *)(v2 + 312)) >> 5);
  v30[7] = sub_8E1790(v4);
  v30[8] = v31;
  v30[10] = 0;
  v30[5] = sub_6832D0(v3) + 1;
  v30[9] = v5;
  _RAX = sub_37BF40(32);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups ymmword ptr [rax], ymm0 }
  *(_QWORD *)(v2 + 344) = _RAX;
  sub_10DC7A0(_RAX, v30);
  v9 = sub_10DCD20(*(_QWORD *)(v2 + 344));
  v10 = v9;
  v11 = HIDWORD(v9);
  if ( byte_1ADF290 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADF290);
    if ( v12 != 0 )
    {
      qword_1ADF288 = sub_37BA70("gpu");
      _cxa_guard_release(&byte_1ADF290);
    }
  }
  sub_37A920(qword_1ADF288);
  *(_QWORD *)(v2 + 352) = sub_37AE70(v10, *(_QWORD *)(v2 + 152), (unsigned int)v11);
  sub_37A9B0(v13, v14);
  if ( *(_QWORD *)(v3 + 24) != 0 )
  {
    v15 = *(_QWORD *)(v2 + 312);
    if ( *(_QWORD *)(v2 + 320) != v15 )
    {
      v26 = v2;
      v16 = 0;
      v23 = v3;
      v17 = v2;
      do
      {
        v24 = v16;
        v22 = 6 * v16;
        v25 = 0;
        while ( 1 )
        {
          if ( sub_6832D0(v3) != -1 )
          {
            v18 = 0;
            v19 = 480 * v24 + v15 + 80 * v25;
            v20 = v25 + v22;
            do
            {
              sub_10FC210(v29, *(_QWORD *)(v17 + 344), (unsigned int)v18, v20);
              sub_10FD200(&v28, &v27, *(_QWORD *)(v17 + 344), (unsigned int)v18, v20);
              sub_10FDB10(v28 + *(_QWORD *)(v17 + 352), *(_QWORD *)(v19 + 24), v29);
              v19 = *(_QWORD *)(v19 + 40);
              ++v18;
            }
            while ( v18 < sub_6832D0(v23) + 1 );
          }
          v3 = v23;
          if ( ++v25 == 6 )
            break;
          v15 = *(_QWORD *)(v26 + 312);
        }
        v2 = v26;
        v15 = *(_QWORD *)(v26 + 312);
        v16 = v24 + 1;
      }
      while ( v24 + 1 < 0xEEEEEEEEEEEEEEEFLL * ((*(_QWORD *)(v26 + 320) - v15) >> 5) );
    }
  }
  sub_10DDED0(*(_QWORD *)(v2 + 344), *(_QWORD *)(v2 + 352) >> 8);
  sub_10DEA40(*(_QWORD *)(v2 + 344), "so.1");
  return 0x6365786562696C2FLL;
}
