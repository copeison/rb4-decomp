/* Generated Hex-Rays evidence for the Orbis constant-buffer backend. */

/* 0x8E3800 */
// Constructs the common constant buffer and clears the 32-byte Orbis backend state.
void *__fastcall orbis_constant_buffer_construct(_QWORD *a1)
{
  _RBX = a1;
  constant_buffer_construct(a1);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  *_RBX = &unk_195F838;
  __asm { vmovups ymmword ptr [rbx+50h], ymm0 }
  return &unk_195F838;
}


/* 0x8E3830 */
// Releases the persistent GPU copy used by the Orbis constant buffer.
void __fastcall orbis_constant_buffer_destruct(_QWORD *a1)
{
  __int64 v1; // rax
  __int64 v3; // rsi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195F838;
  v3 = a1[11];
  if ( v3 != 0 )
  {
    orbis_defer_allocation_release(g_orbis_render_system, v3);
    a1[11] = 0;
  }
  a1[12] = 0;
}


/* 0x8E3880 */
// Shared Orbis constant-buffer backend release helper.
void __fastcall orbis_constant_buffer_release_backend(__int64 a1)
{
  __int64 v1; // rax
  __int64 v3; // rsi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  v3 = *(_QWORD *)(a1 + 88);
  if ( v3 != 0 )
  {
    orbis_defer_allocation_release(g_orbis_render_system, v3);
    *(_QWORD *)(a1 + 88) = 0;
  }
  *(_QWORD *)(a1 + 96) = 0;
}


/* 0x8E38C0 */
// Deleting Orbis constant-buffer destructor.
double __fastcall orbis_constant_buffer_delete(_QWORD *a1)
{
  __int64 v1; // rax
  __int64 v3; // rsi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  *a1 = &unk_195F838;
  v3 = a1[11];
  if ( v3 != 0 )
    orbis_defer_allocation_release(g_orbis_render_system, v3);
  return sub_37BF50(a1);
}


/* 0x8E3900 */
// Allocates at least 16 bytes of persistent GPU storage and copies the CPU data.
__int64 __fastcall orbis_constant_buffer_initialize_backend(_QWORD *a1)
{
  __int64 v1; // rax
  __int64 v3; // rsi
  char *v4; // rdi
  __int64 v5; // rax
  __int64 result; // rax
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  v3 = a1[11];
  if ( v3 != 0 )
  {
    orbis_defer_allocation_release(g_orbis_render_system, v3);
    a1[11] = 0;
  }
  v4 = "so.1";
  if ( a1[5] != 0 )
    v4 = (char *)(16LL * a1[5]);
  a1[12] = v4;
  v5 = sub_37AE70(v4, "CBuffer", 4);
  a1[11] = v5;
  result = memcpy(v5, a1[6], a1[12]);
  a1[10] = 0;
  return result;
}


/* 0x8E3980 */
// Copies a 16-byte-element range from CPU data into persistent GPU storage.
__int64 __fastcall orbis_constant_buffer_update_range(_QWORD *a1, __int64 a2, __int64 a3, __int64 a4)
{
  __int64 v4; // rax
  __int64 result; // rax
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v4;
  result = memcpy(a1[11] + 16 * a3, a1[6], 16 * (a4 - a3));
  a1[10] = 0;
  return result;
}


/* 0x8E39C0 */
// Uploads frame-local constant data, builds its Gnm descriptor, and binds the requested shader-stage mask.
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_constant_buffer_bind(__int64 a1, __int64 a2)
{
  int v4; // r13d
  __int64 v5; // rax
  unsigned __int64 v6; // rdi
  unsigned __int64 *v7; // r14
  int v8; // eax
  __int64 v9; // rax
  unsigned __int64 v10; // rdi
  unsigned __int64 v11; // r12
  unsigned __int64 *v12; // rdx
  __int64 v13; // rsi
  unsigned __int64 v14; // rax
  __int64 v15; // rcx
  unsigned __int64 v16; // r12
  unsigned __int64 v17; // rdi
  __int64 v18; // rsi
  unsigned __int64 *v19; // r8
  _DWORD *v20; // rbx
  unsigned int v22; // [rsp+Ch] [rbp-54h]
  _DWORD *v23; // [rsp+10h] [rbp-50h]
  unsigned __int64 *v24; // [rsp+18h] [rbp-48h]
  unsigned __int64 *v25; // [rsp+18h] [rbp-48h]
  _QWORD v26[8]; // [rsp+20h] [rbp-40h] BYREF

  v26[2] = 0x6365786562696C2FLL;
  v22 = *(_DWORD *)(a1 + 20);
  v4 = *(_DWORD *)(a1 + 24);
  v5 = *(_QWORD *)(g_render_system + 160);
  if ( *(_QWORD *)(a1 + 104) == v5 )
  {
    v6 = *(_QWORD *)(a1 + 80);
    v7 = (unsigned __int64 *)(a1 + 80);
    if ( v6 != 0 )
    {
      v23 = (_DWORD *)(a2 + 18980);
      goto LABEL_18;
    }
  }
  else
  {
    v7 = (unsigned __int64 *)(a1 + 80);
    *(_QWORD *)(a1 + 104) = v5;
    *(_QWORD *)(a1 + 80) = 0;
  }
  v23 = (_DWORD *)(a2 + 18980);
  v8 = *(_DWORD *)(a2 + 18980);
  if ( v8 == 1 )
  {
    v14 = 6880LL * *(_QWORD *)(a2 + 18984);
    v15 = a2 + 61920LL * *(_QWORD *)(a2 + 265616);
    v16 = ((unsigned __int64)*(unsigned int *)(a1 + 96) + 3) >> 2;
    v17 = *(_QWORD *)((char *)sub_229E8 + v14 + v15);
    v18 = (unsigned int)(v16 + 2);
    v19 = (unsigned __int64 *)((char *)sub_229E8 + v14 + v15);
    if ( (unsigned int)((v17 - *(_QWORD *)((char *)&loc_229F0 + v14 + v15)) >> 2) < (unsigned int)v18 )
    {
      v25 = (unsigned __int64 *)((char *)sub_229E8 + v14 + v15);
      if ( (*(unsigned __int8 (__fastcall **)(unsigned __int64, __int64, _QWORD))((char *)&qword_229F8[v14 / 8] + v15))(
             v14 + v15 + 141792,
             v18,
             *(_QWORD *)((char *)sub_22A00 + v14 + v15)) == 0 )
        goto LABEL_16;
      v19 = v25;
      v17 = *v25;
    }
    v6 = (v17 - 4 * v16) & 0xFFFFFFFFFFFFFFFCLL;
    *v19 = v6;
    goto LABEL_17;
  }
  if ( v8 != 0 )
  {
    v6 = 0;
    goto LABEL_18;
  }
  v9 = 59528LL * *(_QWORD *)(a2 + 265616);
  v10 = *(_QWORD *)(a2 + v9 + 22320);
  v11 = ((unsigned __int64)*(unsigned int *)(a1 + 96) + 3) >> 2;
  v12 = (unsigned __int64 *)(a2 + v9 + 22320);
  v13 = (unsigned int)(v11 + 2);
  if ( (unsigned int)((*(_QWORD *)(a2 + v9 + 22320) - *(_QWORD *)(a2 + v9 + 22328)) >> 2) < (unsigned int)v13 )
  {
    v24 = (unsigned __int64 *)(a2 + v9 + 22320);
    if ( (*(unsigned __int8 (__fastcall **)(__int64, __int64, _QWORD))(a2 + v9 + 22336))(
           a2 + v9 + 22312,
           v13,
           *(_QWORD *)(a2 + v9 + 22344)) == 0 )
    {
LABEL_16:
      v6 = 0;
      goto LABEL_17;
    }
    v12 = v24;
    v10 = *v24;
  }
  v6 = (v10 - 4 * v11) & 0xFFFFFFFFFFFFFFFCLL;
  *v12 = v6;
LABEL_17:
  *v7 = v6;
LABEL_18:
  memcpy(v6, *(_QWORD *)(a1 + 88), *(_QWORD *)(a1 + 96));
  sub_10C20C0(v26, *v7, *(unsigned int *)(a1 + 96));
  gnm_buffer_set_resource_memory_type((__int64)v26, 16);
  v20 = v23;
  if ( *v23 == 0 )
  {
    if ( (v4 & 1) != 0 )
      sub_10E40A0(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 2, v22, v26);
    if ( (v4 & 2) != 0 )
    {
      sub_10E40A0(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 5, v22, v26);
      v20 = v23;
      sub_10E40A0(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 6, v22, v26);
    }
    if ( (v4 & 4) != 0 )
      sub_10E40A0(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 3, v22, v26);
    if ( (v4 & 8) != 0 )
      sub_10E40A0(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 1, v22, v26);
  }
  if ( (v4 & 0x10) != 0 )
  {
    if ( *v20 == 1 )
    {
      sub_10EEDA0(6880LL * *(_QWORD *)(a2 + 18984) + a2 + 61920LL * *(_QWORD *)(a2 + 265616) + 141856, v22, 1, v26);
    }
    else if ( *v20 == 0 )
    {
      sub_10E40A0(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 0, v22, v26);
    }
  }
  return 0x6365786562696C2FLL;
}

