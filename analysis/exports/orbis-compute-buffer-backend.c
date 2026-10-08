/* Generated Hex-Rays evidence for the Orbis compute-buffer backend. */

/* 0x8E3290 */
__int64 __fastcall orbis_compute_buffer_destruct(_QWORD *a1)
{
  double v2; // xmm0_8

  *a1 = &unk_195F7B8;
  orbis_defer_allocation_release(g_orbis_render_system, a1[14]);
  a1[14] = 0;
  v2 = orbis_defer_allocation_release(g_orbis_render_system, a1[15]);
  a1[15] = 0;
  return sub_636D10(a1, v2);
}

/* 0x8E32F0 */
double __fastcall orbis_compute_buffer_delete(_QWORD *a1)
{
  double v2; // xmm0_8

  *a1 = &unk_195F7B8;
  orbis_defer_allocation_release(g_orbis_render_system, a1[14]);
  a1[14] = 0;
  v2 = orbis_defer_allocation_release(g_orbis_render_system, a1[15]);
  a1[15] = 0;
  sub_636D10(a1, v2);
  return sub_37BF50(a1);
}

/* 0x8E3350 */
char __fastcall orbis_compute_buffer_initialize_backend(__int64 a1)
{
  __int64 v1; // rax
  __int64 *v3; // r15
  __int64 v5; // rbx
  __int64 v6; // r13
  __int64 v7; // r12
  unsigned int v8; // r12d
  int v9; // eax
  __int64 v10; // rdi
  __int64 v11; // rsi
  __int64 v12; // rsi
  char v14; // si
  __int64 savedregs; // [rsp+28h] [rbp+0h]

  savedregs = v1;
  v3 = (__int64 *)(a1 + 112);
  orbis_defer_allocation_release(g_orbis_render_system, *(_QWORD *)(a1 + 112));
  *(_QWORD *)(a1 + 112) = 0;
  *(double *)&_XMM0 = orbis_defer_allocation_release(g_orbis_render_system, *(_QWORD *)(a1 + 120));
  *(_QWORD *)(a1 + 120) = 0;
  v5 = a1 + 80;
  v6 = ((*(_DWORD *)(a1 + 52) & 0x10) != 0) + 1LL;
  do
  {
    v7 = *(_QWORD *)(a1 + 40);
    if ( v7 == 0 )
    {
      v8 = ((*(_DWORD *)(a1 + 52) >> 1) & 4) + 4;
      if ( byte_1ADF148 == 0 )
      {
        _cxa_guard_acquire(&byte_1ADF148);
        if ( v9 != 0 )
        {
          qword_1ADF140 = sub_37BA70("gpu");
          _cxa_guard_release(&byte_1ADF148);
        }
      }
      sub_37A920(qword_1ADF140);
      v7 = sub_37AE70(*(_QWORD *)(a1 + 24) * *(_QWORD *)(a1 + 16), "ComputeBuffer", v8);
      *v3 = v7;
      sub_37A9B0(v10, v11);
      v12 = *(_QWORD *)(a1 + 32);
      if ( v12 != 0 )
      {
        memcpy(*v3, v12, *(_QWORD *)(a1 + 24) * *(_QWORD *)(a1 + 16));
      }
      else if ( (*(_BYTE *)(a1 + 52) & 8) != 0 )
      {
        _RAX = *v3;
        __asm
        {
          vmovups xmm0, cs:xmmword_12D1730
          vmovups xmmword ptr [rax], xmm0
        }
      }
    }
    sub_10C1FF0(v5, v7, *(unsigned int *)(a1 + 16), *(unsigned int *)(a1 + 24), *(double *)&_XMM0);
    if ( (*(_BYTE *)(a1 + 52) & 9) != 0 )
      v14 = 109;
    else
      v14 = 16;
    gnm_buffer_set_resource_memory_type(v5, v14);
    ++v3;
    v5 += 16;
    --v6;
  }
  while ( v6 != 0 );
  return 1;
}

/* 0x8E34D0 */
void __fastcall orbis_compute_buffer_release_backend(__int64 a1)
{
  orbis_defer_allocation_release(g_orbis_render_system, *(_QWORD *)(a1 + 112));
  *(_QWORD *)(a1 + 112) = 0;
  orbis_defer_allocation_release(g_orbis_render_system, *(_QWORD *)(a1 + 120));
  *(_QWORD *)(a1 + 120) = 0;
}

/* 0x8E3510 */
__int64 __fastcall orbis_compute_buffer_update_gpu_data(__int64 a1)
{
  _BOOL8 v1; // rax

  v1 = (*(_DWORD *)(a1 + 128) & 1) == 0;
  *(_QWORD *)(a1 + 128) = v1;
  return memcpy(*(_QWORD *)(a1 + 8 * v1 + 112), *(_QWORD *)(a1 + 64), *(_QWORD *)(a1 + 72));
}

/* 0x8E3580 */
__int64 __fastcall orbis_compute_buffer_bind_vertex(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 v3; // rax
  unsigned int v6; // r14d
  __int64 savedregs; // [rsp+18h] [rbp+0h]

  savedregs = v3;
  v6 = a3;
  sub_10E3600(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 4, a3, a1 + 16LL * *(_QWORD *)(a1 + 128) + 80);
  return sub_10E3600(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 2, v6, a1 + 16LL * *(_QWORD *)(a1 + 128) + 80);
}

/* 0x8E3600 */
__int64 __fastcall orbis_compute_buffer_bind_hull(__int64 a1, __int64 a2, __int64 a3)
{
  return sub_10E3600(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 5, a3, a1 + 16LL * *(_QWORD *)(a1 + 128) + 80);
}

/* 0x8E3630 */
__int64 __fastcall orbis_compute_buffer_bind_domain(__int64 a1, __int64 a2, __int64 a3)
{
  return sub_10E3600(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 6, a3, a1 + 16LL * *(_QWORD *)(a1 + 128) + 80);
}

/* 0x8E3660 */
__int64 __fastcall orbis_compute_buffer_bind_geometry(__int64 a1, __int64 a2, __int64 a3)
{
  return sub_10E3600(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 3, a3, a1 + 16LL * *(_QWORD *)(a1 + 128) + 80);
}

/* 0x8E3690 */
__int64 __fastcall orbis_compute_buffer_bind_pixel(__int64 a1, __int64 a2, __int64 a3, char a4)
{
  _QWORD *v5; // rcx
  __int64 v6; // rdi

  v5 = (_QWORD *)(a1 + 16LL * *(_QWORD *)(a1 + 128) + 80);
  v6 = a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056;
  if ( (a4 & 1) != 0 )
    return gnmx_constant_update_engine_set_rw_buffer(v6, 1u, a3, v5);
  else
    return sub_10E3600(v6, 1, a3, v5);
}

/* 0x8E36D0 */
__int64 __fastcall orbis_compute_buffer_bind_compute(__int64 a1, __int64 a2, __int64 a3, char a4)
{
  __int64 result; // rax
  int v5; // edx

  result = a3;
  v5 = *(_DWORD *)(a2 + 18980);
  if ( (a4 & 1) != 0 )
  {
    if ( v5 == 1 )
      JUMPOUT(0x10EEF40);
    if ( v5 == 0 )
      return gnmx_constant_update_engine_set_rw_buffer(
               a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056,
               0,
               result,
               (_QWORD *)(a1 + 16LL * *(_QWORD *)(a1 + 128) + 80));
  }
  else
  {
    if ( v5 == 1 )
      JUMPOUT(0x10EEE70);
    if ( v5 == 0 )
      return sub_10E3600(
               a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056,
               0,
               (unsigned int)result,
               a1 + 16LL * *(_QWORD *)(a1 + 128) + 80);
  }
  return result;
}
