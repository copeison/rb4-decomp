/* Generated Hex-Rays evidence for the Orbis 1D texture backend. */

/* 0x8E4F90 */
__int64 __fastcall orbis_texture_1d_destruct(_QWORD *a1)
{
  __int64 v1; // rax
  __int64 v3; // rdi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195F970;
  orbis_defer_allocation_release(g_orbis_render_system, a1[50]);
  v3 = a1[49];
  if ( v3 != 0 )
    sub_37BF50(v3);
  a1[49] = 0;
  return sub_6F5970(a1);
}

/* 0x8E4FF0 */
double __fastcall orbis_texture_1d_delete(_QWORD *a1)
{
  __int64 v1; // rax
  __int64 v3; // rdi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195F970;
  orbis_defer_allocation_release(g_orbis_render_system, a1[50]);
  v3 = a1[49];
  if ( v3 != 0 )
    sub_37BF50(v3);
  a1[49] = 0;
  sub_6F5970(a1);
  return sub_37BF50(a1);
}

/* 0x8E5050 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_texture_1d_initialize_backend(__int64 a1)
{
  __int64 v3; // r14
  __int64 v4; // rbx
  unsigned int v5; // r13d
  unsigned int v6; // r12d
  unsigned int v7; // eax
  __int64 v8; // r13
  unsigned __int64 v11; // rax
  unsigned int v12; // r14d
  unsigned __int64 v13; // rbx
  int v14; // eax
  __int64 v15; // rdi
  __int64 v16; // rsi
  __int64 v17; // r14
  unsigned __int64 v18; // rbx
  __int64 v20; // [rsp+8h] [rbp-A8h] BYREF
  __int64 v21; // [rsp+10h] [rbp-A0h] BYREF
  _BYTE v22[48]; // [rsp+18h] [rbp-98h] BYREF
  _DWORD v23[13]; // [rsp+48h] [rbp-68h] BYREF
  int v24; // [rsp+7Ch] [rbp-34h] BYREF
  __int64 v25; // [rsp+80h] [rbp-30h]

  v3 = a1 + 312;
  v4 = a1 + 64;
  v25 = 0x6365786562696C2FLL;
  v5 = *(_DWORD *)(a1 + 332);
  v6 = sub_10DEF20();
  LODWORD(v4) = sub_8E1820(v4);
  v7 = sub_8E1790(v5);
  sub_10F4A00(v6, &v24, (unsigned int)v4, v7, 1);
  sub_10DC780(v23);
  v23[0] = 8;
  v23[1] = *(_DWORD *)(a1 + 112);
  v23[2] = 1;
  v23[3] = 1;
  v23[4] = 0;
  v23[6] = 1;
  v23[7] = sub_8E1790(v5);
  v8 = v3;
  v23[8] = v24;
  v23[10] = 0;
  v23[5] = sub_6832D0(v3) + 1;
  v23[9] = v6;
  _RAX = sub_37BF40(32);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups ymmword ptr [rax], ymm0 }
  *(_QWORD *)(a1 + 392) = _RAX;
  sub_10DC7A0(_RAX, v23);
  v11 = sub_10DCD20(*(_QWORD *)(a1 + 392));
  v12 = v11;
  v13 = HIDWORD(v11);
  if ( byte_1ADF200 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADF200);
    if ( v14 != 0 )
    {
      qword_1ADF1F8 = sub_37BA70("gpu");
      _cxa_guard_release(&byte_1ADF200);
    }
  }
  sub_37A920(qword_1ADF1F8);
  *(_QWORD *)(a1 + 400) = sub_37AE70(v12, *(_QWORD *)(a1 + 152), (unsigned int)v13);
  sub_37A9B0(v15, v16);
  v17 = v8;
  if ( *(_QWORD *)(a1 + 336) != 0 && sub_6832D0(v8) != -1 )
  {
    v18 = 0;
    do
    {
      sub_10FC210(v22, *(_QWORD *)(a1 + 392), (unsigned int)v18, 0);
      sub_10FD200(&v21, &v20, *(_QWORD *)(a1 + 392), (unsigned int)v18, 0);
      sub_10FDB10(v21 + *(_QWORD *)(a1 + 400), *(_QWORD *)(v8 + 24), v22);
      v8 = *(_QWORD *)(v8 + 40);
      ++v18;
    }
    while ( v18 < sub_6832D0(v17) + 1 );
  }
  sub_10DDED0(*(_QWORD *)(a1 + 392), *(_QWORD *)(a1 + 400) >> 8);
  sub_10DEA40(*(_QWORD *)(a1 + 392), "so.1");
  return 0x6365786562696C2FLL;
}
