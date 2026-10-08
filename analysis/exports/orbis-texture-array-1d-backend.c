/* Generated Hex-Rays evidence for the Orbis 1D texture-array backend. */

/* 0x8E58A0 */
__int64 __fastcall orbis_texture_array_1d_destruct(_QWORD *a1)
{
  __int64 v1; // rax
  double v3; // xmm0_8
  __int64 v4; // rdi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195FAE0;
  v3 = orbis_defer_allocation_release(g_orbis_render_system, a1[44]);
  v4 = a1[43];
  if ( v4 != 0 )
    v3 = sub_37BF50(v4);
  a1[43] = 0;
  return sub_697020(a1, v3);
}

/* 0x8E5900 */
double __fastcall orbis_texture_array_1d_delete(_QWORD *a1)
{
  __int64 v1; // rax
  double v3; // xmm0_8
  __int64 v4; // rdi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195FAE0;
  v3 = orbis_defer_allocation_release(g_orbis_render_system, a1[44]);
  v4 = a1[43];
  if ( v4 != 0 )
    v3 = sub_37BF50(v4);
  a1[43] = 0;
  sub_697020(a1, v3);
  return sub_37BF50(a1);
}

/* 0x8E5960 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_texture_array_1d_initialize_backend(__int64 a1)
{
  __int64 v3; // rbx
  __int64 v4; // r14
  unsigned int v5; // r13d
  unsigned int v6; // r12d
  unsigned int v7; // eax
  __int64 v8; // rdi
  __int64 v9; // r13
  unsigned __int64 v12; // rax
  unsigned int v13; // r14d
  unsigned __int64 v14; // rbx
  int v15; // eax
  __int64 v16; // rdi
  __int64 v17; // rsi
  __int64 v18; // r14
  unsigned __int64 v19; // rbx
  unsigned __int64 v20; // r13
  __int64 v21; // r14
  __int64 v23; // [rsp+0h] [rbp-B0h]
  __int64 v24; // [rsp+8h] [rbp-A8h] BYREF
  __int64 v25; // [rsp+10h] [rbp-A0h] BYREF
  _BYTE v26[48]; // [rsp+18h] [rbp-98h] BYREF
  _DWORD v27[13]; // [rsp+48h] [rbp-68h] BYREF
  int v28; // [rsp+7Ch] [rbp-34h] BYREF
  __int64 v29; // [rsp+80h] [rbp-30h]

  v3 = a1 + 64;
  v29 = 0x6365786562696C2FLL;
  v4 = *(_QWORD *)(a1 + 312);
  v5 = *(_DWORD *)(v4 + 20);
  v6 = sub_10DEF20();
  LODWORD(v3) = sub_8E1820(v3);
  v7 = sub_8E1790(v5);
  sub_10F4A00(v6, &v28, (unsigned int)v3, v7, 1);
  sub_10DC780(v27);
  v27[0] = 12;
  v8 = v5;
  v9 = v4;
  v27[1] = *(_DWORD *)(a1 + 112);
  v27[2] = 1;
  v27[3] = 1;
  v27[4] = 0;
  v27[6] = -858993459 * ((*(_QWORD *)(a1 + 320) - *(_QWORD *)(a1 + 312)) >> 4);
  v27[7] = sub_8E1790(v8);
  v27[8] = v28;
  v27[10] = 0;
  v27[5] = sub_6832D0(v4) + 1;
  v27[9] = v6;
  _RAX = sub_37BF40(32);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups ymmword ptr [rax], ymm0 }
  *(_QWORD *)(a1 + 344) = _RAX;
  sub_10DC7A0(_RAX, v27);
  v12 = sub_10DCD20(*(_QWORD *)(a1 + 344));
  v13 = v12;
  v14 = HIDWORD(v12);
  if ( byte_1ADF240 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADF240);
    if ( v15 != 0 )
    {
      qword_1ADF238 = sub_37BA70("gpu");
      _cxa_guard_release(&byte_1ADF240);
    }
  }
  sub_37A920(qword_1ADF238);
  *(_QWORD *)(a1 + 352) = sub_37AE70(v13, *(_QWORD *)(a1 + 152), (unsigned int)v14);
  sub_37A9B0(v16, v17);
  if ( *(_QWORD *)(v9 + 24) != 0 )
  {
    v18 = *(_QWORD *)(a1 + 312);
    if ( *(_QWORD *)(a1 + 320) != v18 )
    {
      v19 = 0;
      v23 = v9;
      do
      {
        if ( sub_6832D0(v9) != -1 )
        {
          v20 = 0;
          v21 = 80 * v19 + v18;
          do
          {
            sub_10FC210(v26, *(_QWORD *)(a1 + 344), (unsigned int)v20, (unsigned int)v19);
            sub_10FD200(&v25, &v24, *(_QWORD *)(a1 + 344), (unsigned int)v20, (unsigned int)v19);
            sub_10FDB10(v25 + *(_QWORD *)(a1 + 352), *(_QWORD *)(v21 + 24), v26);
            v21 = *(_QWORD *)(v21 + 40);
            ++v20;
          }
          while ( v20 < sub_6832D0(v23) + 1 );
        }
        v18 = *(_QWORD *)(a1 + 312);
        v9 = v23;
        ++v19;
      }
      while ( v19 < 0xCCCCCCCCCCCCCCCDLL * ((*(_QWORD *)(a1 + 320) - v18) >> 4) );
    }
  }
  sub_10DDED0(*(_QWORD *)(a1 + 344), *(_QWORD *)(a1 + 352) >> 8);
  sub_10DEA40(*(_QWORD *)(a1 + 344), "so.1");
  return 0x6365786562696C2FLL;
}
