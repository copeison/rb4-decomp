/* Generated Hex-Rays evidence for shared Orbis texture binding. */

/* 0x8E1EE0 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_bind_vertex_texture(__int64 a1, char *a2, __int64 a3, int a4, int a5, __m128 a6)
{
  unsigned int v6; // r15d
  _QWORD v10[8]; // [rsp+0h] [rbp-40h] BYREF

  v6 = (unsigned int)a2;
  v10[2] = 0x6365786562696C2FLL;
  if ( (unsigned __int64)a2 < 0x10 )
  {
    orbis_build_sampler_descriptor((__int64)v10, a4, a5, a6);
    gnmx_gfx_context_set_samplers(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 2u, v6, v10);
  }
  sub_10E3050(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 2, v6, a3);
  return 0x6365786562696C2FLL;
}

/* 0x8E1F90 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_bind_hull_texture(
        __int64 a1,
        unsigned __int64 a2,
        __int64 a3,
        int a4,
        int a5,
        char a6,
        __m128 a7)
{
  unsigned int v7; // r15d
  _QWORD v11[8]; // [rsp+0h] [rbp-40h] BYREF

  v7 = a2;
  v11[2] = 0x6365786562696C2FLL;
  if ( a2 <= 0xF && (a6 & 0x10) == 0 )
  {
    orbis_build_sampler_descriptor((__int64)v11, a4, a5, a7);
    gnmx_gfx_context_set_samplers(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 5u, v7, v11);
  }
  sub_10E3050(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 5, v7, a3);
  return 0x6365786562696C2FLL;
}

/* 0x8E2040 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_bind_domain_texture(
        __int64 a1,
        unsigned __int64 a2,
        __int64 a3,
        int a4,
        int a5,
        char a6,
        __m128 a7)
{
  unsigned int v7; // r15d
  _QWORD v11[8]; // [rsp+0h] [rbp-40h] BYREF

  v7 = a2;
  v11[2] = 0x6365786562696C2FLL;
  if ( a2 <= 0xF && (a6 & 0x10) == 0 )
  {
    orbis_build_sampler_descriptor((__int64)v11, a4, a5, a7);
    gnmx_gfx_context_set_samplers(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 6u, v7, v11);
  }
  sub_10E3050(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 6, v7, a3);
  return 0x6365786562696C2FLL;
}

/* 0x8E20F0 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_bind_geometry_texture(
        __int64 a1,
        unsigned __int64 a2,
        __int64 a3,
        int a4,
        int a5,
        char a6,
        __m128 a7)
{
  unsigned int v7; // r15d
  _QWORD v11[8]; // [rsp+0h] [rbp-40h] BYREF

  v7 = a2;
  v11[2] = 0x6365786562696C2FLL;
  if ( a2 <= 0xF && (a6 & 0x10) == 0 )
  {
    orbis_build_sampler_descriptor((__int64)v11, a4, a5, a7);
    gnmx_gfx_context_set_samplers(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 3u, v7, v11);
  }
  sub_10E3050(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 3, v7, a3);
  return 0x6365786562696C2FLL;
}

/* 0x8E21A0 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_bind_pixel_texture(
        __int64 a1,
        unsigned __int64 a2,
        __int64 a3,
        int a4,
        int a5,
        char a6,
        __m128 a7)
{
  unsigned int v8; // r15d
  _QWORD v11[8]; // [rsp+0h] [rbp-40h] BYREF

  v8 = a2;
  v11[2] = 0x6365786562696C2FLL;
  if ( (a6 & 1) != 0 )
    JUMPOUT(0x10E45F0);
  if ( a2 <= 0xF && (a6 & 0x10) == 0 )
  {
    orbis_build_sampler_descriptor((__int64)v11, a4, a5, a7);
    gnmx_gfx_context_set_samplers(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 1u, v8, v11);
  }
  sub_10E3050(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 1, v8, a3);
  return 0x6365786562696C2FLL;
}

/* 0x8E2290 */
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_bind_compute_texture(__int64 a1, char *a2, __int64 a3, int a4, int a5, char a6, __m128 a7)
{
  unsigned int v8; // r14d
  bool v10; // r13
  int v11; // eax
  int v12; // eax
  _QWORD v14[8]; // [rsp+0h] [rbp-40h] BYREF

  v8 = (unsigned int)a2;
  v14[2] = 0x6365786562696C2FLL;
  if ( (a6 & 1) != 0 )
  {
    v12 = *(_DWORD *)(a1 + 18980);
    if ( v12 == 1 )
      JUMPOUT(0x10EF0E0);
    if ( v12 == 0 )
      JUMPOUT(0x10E45F0);
  }
  else
  {
    v10 = (unsigned __int64)a2 < 0x10 && (a6 & 0x10) == 0;
    if ( v10 )
      orbis_build_sampler_descriptor((__int64)v14, a4, a5, a7);
    v11 = *(_DWORD *)(a1 + 18980);
    if ( v11 == 1 )
    {
      sub_10EF010(6880LL * *(_QWORD *)(a1 + 18984) + a1 + 61920LL * *(_QWORD *)(a1 + 265616) + 141856, v8, 1, a3);
      if ( v10 )
        gnmx_compute_context_set_samplers(
          6880LL * *(_QWORD *)(a1 + 18984) + a1 + 61920LL * *(_QWORD *)(a1 + 265616) + 141856,
          v8,
          1,
          (__int64)v14);
    }
    else if ( v11 == 0 )
    {
      sub_10E3050(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 0, v8, a3);
      if ( v10 )
        gnmx_gfx_context_set_samplers(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056, 0, v8, v14);
    }
  }
  return 0x6365786562696C2FLL;
}
