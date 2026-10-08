/* Generated Hex-Rays evidence for Orbis 2D texture binding. */

/* 0x8D6E40 */
__int64 __fastcall orbis_texture_2d_bind_vertex(__int64 a1, __int64 a2, char *a3, char a4, __m128 a5)
{
  __int64 v5; // rax
  __int64 *v6; // rax

  if ( (a4 & 4) != 0 )
  {
    v6 = (__int64 *)(a1 + 424);
  }
  else
  {
    if ( *(_QWORD *)(a1 + 488) != 0 )
    {
      v5 = *(_QWORD *)(*(_QWORD *)(g_orbis_render_system + 112) + 32LL);
      if ( *(_QWORD *)(a1 + 8 * v5 + 488) == 0 )
        v5 = 0;
    }
    else
    {
      v5 = *(_QWORD *)(a1 + 456);
    }
    v6 = (__int64 *)(a1 + 8 * v5 + 408);
  }
  return orbis_bind_vertex_texture(a2, a3, *v6, *(_DWORD *)(a1 + 96), *(_DWORD *)(a1 + 100), a5);
}

/* 0x8D6F10 */
__int64 __fastcall orbis_texture_2d_bind_hull(__int64 a1, __int64 a2, unsigned __int64 a3, char a4, __m128 a5)
{
  __int64 v5; // rax
  __int64 *v6; // rax

  if ( (a4 & 4) != 0 )
  {
    v6 = (__int64 *)(a1 + 424);
  }
  else
  {
    if ( *(_QWORD *)(a1 + 488) != 0 )
    {
      v5 = *(_QWORD *)(*(_QWORD *)(g_orbis_render_system + 112) + 32LL);
      if ( *(_QWORD *)(a1 + 8 * v5 + 488) == 0 )
        v5 = 0;
    }
    else
    {
      v5 = *(_QWORD *)(a1 + 456);
    }
    v6 = (__int64 *)(a1 + 8 * v5 + 408);
  }
  return orbis_bind_hull_texture(a2, a3, *v6, *(_DWORD *)(a1 + 96), *(_DWORD *)(a1 + 100), a4, a5);
}

/* 0x8D6F80 */
__int64 __fastcall orbis_texture_2d_bind_domain(__int64 a1, __int64 a2, unsigned __int64 a3, char a4, __m128 a5)
{
  __int64 v5; // rax
  __int64 *v6; // rax

  if ( (a4 & 4) != 0 )
  {
    v6 = (__int64 *)(a1 + 424);
  }
  else
  {
    if ( *(_QWORD *)(a1 + 488) != 0 )
    {
      v5 = *(_QWORD *)(*(_QWORD *)(g_orbis_render_system + 112) + 32LL);
      if ( *(_QWORD *)(a1 + 8 * v5 + 488) == 0 )
        v5 = 0;
    }
    else
    {
      v5 = *(_QWORD *)(a1 + 456);
    }
    v6 = (__int64 *)(a1 + 8 * v5 + 408);
  }
  return orbis_bind_domain_texture(a2, a3, *v6, *(_DWORD *)(a1 + 96), *(_DWORD *)(a1 + 100), a4, a5);
}

/* 0x8D6FF0 */
__int64 __fastcall orbis_texture_2d_bind_geometry(__int64 a1, __int64 a2, unsigned __int64 a3, char a4, __m128 a5)
{
  __int64 v5; // rax
  __int64 *v6; // rax

  if ( (a4 & 4) != 0 )
  {
    v6 = (__int64 *)(a1 + 424);
  }
  else
  {
    if ( *(_QWORD *)(a1 + 488) != 0 )
    {
      v5 = *(_QWORD *)(*(_QWORD *)(g_orbis_render_system + 112) + 32LL);
      if ( *(_QWORD *)(a1 + 8 * v5 + 488) == 0 )
        v5 = 0;
    }
    else
    {
      v5 = *(_QWORD *)(a1 + 456);
    }
    v6 = (__int64 *)(a1 + 8 * v5 + 408);
  }
  return orbis_bind_geometry_texture(a2, a3, *v6, *(_DWORD *)(a1 + 96), *(_DWORD *)(a1 + 100), a4, a5);
}

/* 0x8D7060 */
__int64 __fastcall orbis_texture_2d_bind_pixel(__int64 a1, __int64 a2, unsigned __int64 a3, char a4, __m128 a5)
{
  __int64 v5; // rax
  __int64 *v6; // rax

  if ( (a4 & 4) != 0 )
  {
    v6 = (__int64 *)(a1 + 424);
  }
  else
  {
    if ( *(_QWORD *)(a1 + 488) != 0 )
    {
      v5 = *(_QWORD *)(*(_QWORD *)(g_orbis_render_system + 112) + 32LL);
      if ( *(_QWORD *)(a1 + 8 * v5 + 488) == 0 )
        v5 = 0;
    }
    else
    {
      v5 = *(_QWORD *)(a1 + 456);
    }
    v6 = (__int64 *)(a1 + 8 * v5 + 408);
  }
  return orbis_bind_pixel_texture(a2, a3, *v6, *(_DWORD *)(a1 + 96), *(_DWORD *)(a1 + 100), a4, a5);
}

/* 0x8D70D0 */
__int64 __fastcall orbis_texture_2d_bind_compute(__int64 a1, __int64 a2, char *a3, char a4, __m128 a5)
{
  __int64 v5; // rax
  __int64 *v6; // rax

  if ( (a4 & 4) != 0 )
  {
    v6 = (__int64 *)(a1 + 424);
  }
  else
  {
    if ( *(_QWORD *)(a1 + 488) != 0 )
    {
      v5 = *(_QWORD *)(*(_QWORD *)(g_orbis_render_system + 112) + 32LL);
      if ( *(_QWORD *)(a1 + 8 * v5 + 488) == 0 )
        v5 = 0;
    }
    else
    {
      v5 = *(_QWORD *)(a1 + 456);
    }
    v6 = (__int64 *)(a1 + 8 * v5 + 408);
  }
  return orbis_bind_compute_texture(a2, a3, *v6, *(_DWORD *)(a1 + 96), *(_DWORD *)(a1 + 100), a4, a5);
}
