/* Generated Hex-Rays evidence for Orbis compute and pixel shader backends. */

/* 0x8E3D50 */
__int64 __fastcall orbis_compute_shader_destruct(_QWORD *a1)
{
  *a1 = &unk_195F870;
  sub_642310(a1);
  return nullsub_48(a1);
}

/* 0x8E3D80 */
double __fastcall orbis_compute_shader_delete(_QWORD *a1)
{
  *a1 = &unk_195F870;
  sub_642310(a1);
  nullsub_48(a1);
  return sub_37BF50(a1);
}

/* 0x8E3DC0 */
char __fastcall orbis_compute_shader_initialize(_QWORD *a1, __int64 a2)
{
  double v4; // xmm0_8
  double v5; // xmm0_8
  int v6; // eax
  __int64 v7; // rdi
  __int64 v8; // rsi
  __int64 v9; // rax
  unsigned __int64 v10; // rcx
  _QWORD v12[2]; // [rsp+8h] [rbp-58h] BYREF
  unsigned int v13; // [rsp+18h] [rbp-48h]
  _QWORD v14[8]; // [rsp+20h] [rbp-40h] BYREF

  v14[2] = 0x6365786562696C2FLL;
  v4 = sub_11B2EE0(v14);
  sub_37AA30(v12, 1, 1, v4);
  v5 = sub_11B2F30(v14, a2);
  sub_37AAF0(v12, v5);
  sub_10EF2E0(v12, v14[0]);
  if ( byte_1ADF170 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADF170);
    if ( v6 != 0 )
    {
      qword_1ADF168 = sub_37BA70("gpu");
      _cxa_guard_release(&byte_1ADF170);
    }
  }
  sub_37A920(qword_1ADF168);
  a1[7] = sub_37AE70(v13, "CShader", 256);
  sub_37A9B0(v7, v8);
  a1[6] = sub_37AE70((*(unsigned __int16 *)(v12[0] + 36LL) & 0xFCu) + 4 * HIBYTE(*(_DWORD *)v12[0]) + 40, "CShader", 4);
  memcpy(a1[7], v12[1], v13);
  memcpy(a1[6], v12[0], (*(_WORD *)(v12[0] + 36LL) & 0xFCu) + 4 * HIBYTE(*(_DWORD *)v12[0]) + 40);
  v9 = a1[6];
  a1[5] = v9;
  v10 = a1[7];
  *(_DWORD *)(v9 + 8) = v10 >> 8;
  *(_DWORD *)(v9 + 12) = v10 >> 40;
  sub_11B2F00(v14);
  nullsub_240(v14);
  return 1;
}

/* 0x8E3F40 */
None

/* 0x8E4030 */
void __fastcall orbis_compute_shader_release_backend(_QWORD *a1)
{
  orbis_defer_allocation_release(g_orbis_render_system, a1[6]);
  orbis_defer_allocation_release(g_orbis_render_system, a1[7]);
  a1[5] = 0;
}

/* 0x8E4070 */
__int64 orbis_compute_shader_stage()
{
  return 5;
}

/* 0x8E4410 */
__int64 __fastcall orbis_pixel_shader_destruct(_QWORD *a1)
{
  *a1 = &unk_195F8F0;
  sub_642310(a1);
  return nullsub_48(a1);
}

/* 0x8E4440 */
double __fastcall orbis_pixel_shader_delete(_QWORD *a1)
{
  *a1 = &unk_195F8F0;
  sub_642310(a1);
  nullsub_48(a1);
  return sub_37BF50(a1);
}

/* 0x8E4480 */
char __fastcall orbis_pixel_shader_initialize(_QWORD *a1, __int64 a2)
{
  double v4; // xmm0_8
  double v5; // xmm0_8
  int v6; // eax
  __int64 v7; // rdi
  __int64 v8; // rsi
  __int64 v9; // rax
  unsigned __int64 v10; // rcx
  _QWORD v12[2]; // [rsp+8h] [rbp-58h] BYREF
  unsigned int v13; // [rsp+18h] [rbp-48h]
  _QWORD v14[8]; // [rsp+20h] [rbp-40h] BYREF

  v14[2] = 0x6365786562696C2FLL;
  v4 = sub_11B2EE0(v14);
  sub_37AA30(v12, 1, 1, v4);
  v5 = sub_11B2F30(v14, a2);
  sub_37AAF0(v12, v5);
  sub_10EF2E0(v12, v14[0]);
  if ( byte_1ADF1B0 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADF1B0);
    if ( v6 != 0 )
    {
      qword_1ADF1A8 = sub_37BA70("gpu");
      _cxa_guard_release(&byte_1ADF1B0);
    }
  }
  sub_37A920(qword_1ADF1A8);
  a1[7] = sub_37AE70(v13, "PShader", 256);
  sub_37A9B0(v7, v8);
  a1[6] = sub_37AE70(
            (2 * *(unsigned __int8 *)(v12[0] + 56LL) + 4 * HIBYTE(*(_DWORD *)v12[0]) + 63) & 0xFFC,
            "PShader",
            4);
  memcpy(a1[7], v12[1], v13);
  memcpy(a1[6], v12[0], (2 * *(unsigned __int8 *)(v12[0] + 56LL) + 4 * HIBYTE(*(_DWORD *)v12[0]) + 63) & 0xFFC);
  v9 = a1[6];
  a1[5] = v9;
  v10 = a1[7];
  *(_DWORD *)(v9 + 8) = v10 >> 8;
  *(_DWORD *)(v9 + 12) = v10 >> 40;
  sub_11B2F00(v14);
  nullsub_240(v14);
  return 1;
}

/* 0x8E4600 */
None

/* 0x8E4670 */
void __fastcall orbis_pixel_shader_release_backend(_QWORD *a1)
{
  orbis_defer_allocation_release(g_orbis_render_system, a1[6]);
  orbis_defer_allocation_release(g_orbis_render_system, a1[7]);
  a1[5] = 0;
}

/* 0x8E46B0 */
__int64 orbis_pixel_shader_stage()
{
  return 4;
}
