/* Generated Hex-Rays evidence for the Orbis 3D texture backend. */

/* 0x8E53F0 */
__int64 __fastcall orbis_texture_3d_destruct(_QWORD *a1)
{
  __int64 v1; // rax
  __int64 v3; // rdi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195FA28;
  orbis_defer_allocation_release(g_orbis_render_system, a1[50]);
  v3 = a1[49];
  if ( v3 != 0 )
    sub_37BF50(v3);
  a1[49] = 0;
  return sub_6F5DB0(a1);
}

/* 0x8E5450 */
double __fastcall orbis_texture_3d_delete(_QWORD *a1)
{
  __int64 v1; // rax
  __int64 v3; // rdi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195FA28;
  orbis_defer_allocation_release(g_orbis_render_system, a1[50]);
  v3 = a1[49];
  if ( v3 != 0 )
    sub_37BF50(v3);
  a1[49] = 0;
  sub_6F5DB0(a1);
  return sub_37BF50(a1);
}

/* 0x8E54B0 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_texture_3d_initialize_backend(__int64 a1, __int64 a2)
{
  __int64 v4; // r15
  __int64 v5; // rbx
  unsigned int v6; // r14d
  unsigned int v7; // r12d
  unsigned int v8; // eax
  __int64 v11; // rbx
  int v12; // eax
  __int64 v13; // rdi
  __int64 v14; // rsi
  unsigned __int64 v15; // rbx
  __int64 v16; // r12
  char *v17; // rsi
  __int64 v20; // [rsp+18h] [rbp-A8h] BYREF
  __int64 v21; // [rsp+20h] [rbp-A0h] BYREF
  _BYTE v22[48]; // [rsp+28h] [rbp-98h] BYREF
  _DWORD v23[13]; // [rsp+58h] [rbp-68h] BYREF
  int v24; // [rsp+8Ch] [rbp-34h] BYREF
  __int64 v25; // [rsp+90h] [rbp-30h]

  v4 = a1 + 312;
  v5 = a1 + 64;
  v25 = 0x6365786562696C2FLL;
  v6 = *(_DWORD *)(a1 + 332);
  v7 = sub_10DEF20();
  LODWORD(v5) = sub_8E1820(v5);
  v8 = sub_8E1790(v6);
  sub_10F4A00(v7, &v24, (unsigned int)v5, v8, 1);
  sub_10DC780(v23);
  v23[0] = 10;
  v23[1] = *(_DWORD *)(a1 + 112);
  v23[2] = *(_DWORD *)(a1 + 116);
  v23[3] = *(_DWORD *)(a1 + 120);
  v23[4] = 0;
  v23[6] = 1;
  v23[7] = sub_8E1790(v6);
  v23[8] = v24;
  v23[10] = 0;
  v23[5] = sub_6832D0(v4) + 1;
  v23[9] = v7;
  _RAX = sub_37BF40(32);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups ymmword ptr [rax], ymm0 }
  *(_QWORD *)(a1 + 392) = _RAX;
  sub_10DC7A0(_RAX, v23);
  v11 = sub_10DCD20(*(_QWORD *)(a1 + 392));
  if ( a2 != 0 )
  {
    *(_QWORD *)(a1 + 400) = *(_QWORD *)(a2 + 400);
  }
  else
  {
    if ( byte_1ADF220 == 0 )
    {
      _cxa_guard_acquire(&byte_1ADF220);
      if ( v12 != 0 )
      {
        qword_1ADF218 = sub_37BA70("gpu");
        _cxa_guard_release(&byte_1ADF220);
      }
    }
    sub_37A920(qword_1ADF218);
    *(_QWORD *)(a1 + 400) = sub_37AE70((unsigned int)v11, *(_QWORD *)(a1 + 152), HIDWORD(v11));
    sub_37A9B0(v13, v14);
  }
  if ( *(_QWORD *)(a1 + 336) != 0 && sub_6832D0(v4) != -1 )
  {
    v15 = 0;
    v16 = v4;
    do
    {
      sub_10FC210(v22, *(_QWORD *)(a1 + 392), (unsigned int)v15, 0);
      sub_10FD200(&v21, &v20, *(_QWORD *)(a1 + 392), (unsigned int)v15, 0);
      sub_10FDB10(v21 + *(_QWORD *)(a1 + 400), *(_QWORD *)(v16 + 24), v22);
      v16 = *(_QWORD *)(v16 + 40);
      ++v15;
    }
    while ( v15 < sub_6832D0(v4) + 1 );
  }
  sub_10DDED0(*(_QWORD *)(a1 + 392), *(_QWORD *)(a1 + 400) >> 8);
  if ( (*(_BYTE *)(a1 + 104) & 2) != 0 )
    v17 = (_BYTE *)(&loc_6C + 1);
  else
    v17 = "so.1";
  sub_10DEA40(*(_QWORD *)(a1 + 392), v17);
  return 0x6365786562696C2FLL;
}
