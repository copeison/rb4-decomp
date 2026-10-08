// orbis_render_context_clear_depth_stencil_target at 0x8E99E0
__int64 __fastcall orbis_render_context_clear_depth_stencil_target(
        int *a1,
        __int64 a2,
        unsigned __int8 a3,
        __m128 _XMM0)
{
  _QWORD *v8; // r15
  int v10; // r14d
  int v16; // eax
  unsigned int stencil_slice_size_bytes; // ebx
  unsigned int v31; // eax
  unsigned int v32; // r13d
  int v35; // eax
  unsigned int v39; // r13d
  unsigned int htile_slice_size_bytes; // eax
  __int64 v41; // rcx
  unsigned int v42; // ebx
  unsigned int v44; // [rsp+0h] [rbp-A0h]
  int v46; // [rsp+Ch] [rbp-94h]
  int v47; // [rsp+10h] [rbp-90h] BYREF
  int v48; // [rsp+18h] [rbp-88h] BYREF
  unsigned int v49; // [rsp+20h] [rbp-80h] BYREF
  int v50; // [rsp+28h] [rbp-78h] BYREF
  __int128 v51; // [rsp+30h] [rbp-70h] BYREF
  int v53; // [rsp+50h] [rbp-50h]
  _QWORD v54[8]; // [rsp+60h] [rbp-40h] BYREF

  v54[2] = 0x6365786562696C2FLL;
  if ( (*(_BYTE *)(a2 + 3) & 0x20) != 0 )
  {
    gnm_draw_command_buffer_emit_event((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], 0x2Cu);
    v10 = *(_DWORD *)(a2 + 32) & 0x7FF;
    v46 = ((*(_DWORD *)(a2 + 32) >> 13) & 0x7FF) - v10 + 1;
    if ( (*(_BYTE *)(a2 + 7) & 0x20) != 0
      && (unsigned int)gnm_depth_target_get_stencil_write_address_256((_DWORD *)a2) != 0 )
    {
      __asm
      {
        vpxor   xmm1, xmm1, xmm1
        vmovdqu [rbp+var_70], xmm1
      }
      if ( byte_1A712C8[0] == 0 )
      {
        *(double *)&_XMM0 = _cxa_guard_acquire(byte_1A712C8);
        __asm { vpxor   xmm1, xmm1, xmm1 }
        if ( v16 != 0 )
        {
          _RAX = &unk_1A712B8;
          __asm
          {
            vxorps  xmm0, xmm0, xmm0
            vmovups xmmword ptr [rax], xmm0
          }
          _cxa_guard_release(byte_1A712C8);
          __asm { vpxor   xmm1, xmm1, xmm1 }
        }
      }
      v53 = 12;
      _EAX = a3 | (a3 << 8) | (a3 << 16) | (a3 << 24);
      __asm { vmovd   xmm0, eax }
      __asm
      {
        vpshufd xmm0, xmm0, 0
        vpblendw xmm1, xmm0, xmm1, 0AAh
        vpsrld  xmm0, xmm0, 10h
        vcvtdq2ps xmm0, xmm0
        vmulps  xmm0, xmm0, cs:xmmword_12D18B0
        vcvtdq2ps xmm1, xmm1
      }
      __asm
      {
        vaddps  xmm0, xmm0, xmm1
        vmovups [rbp+var_60], xmm0
      }
      v44 = sub_6372B0(*(_QWORD *)(g_render_system + 3096), (__int64)a1, (__int64)&v51);
      stencil_slice_size_bytes = gnm_depth_target_get_stencil_slice_size_bytes(a2);
      v31 = gnm_depth_target_get_stencil_write_address_256((_DWORD *)a2);
      v32 = v46 * (stencil_slice_size_bytes >> 2);
      sub_10C1F00(v54, v10 * stencil_slice_size_bytes + ((unsigned __int64)v31 << 8), 17412, v32);
      gnm_buffer_set_resource_memory_type((__int64)v54, 109);
      gnmx_constant_update_engine_set_rw_buffer((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5764], 0, v44, v54);
      *(double *)&_XMM0 = (*(double (__fastcall **)(int *, _QWORD, __int64, __int64))(*(_QWORD *)a1 + 152LL))(
                            a1,
                            (v32 >> 6 << 6 < v32) + (v32 >> 6),
                            1,
                            1);
    }
    __asm
    {
      vxorps  xmm0, xmm0, xmm0
      vmovups [rbp+var_70], xmm0
    }
    if ( byte_1A712C8[0] == 0 )
    {
      *(double *)&_XMM0 = _cxa_guard_acquire(byte_1A712C8);
      __asm { vxorps  xmm0, xmm0, xmm0 }
      if ( v35 != 0 )
      {
        _RAX = &unk_1A712B8;
        __asm
        {
          vxorps  xmm0, xmm0, xmm0
          vmovups xmmword ptr [rax], xmm0
        }
        *(double *)&_XMM0 = _cxa_guard_release(byte_1A712C8);
        __asm { vxorps  xmm0, xmm0, xmm0 }
      }
    }
    v53 = 12;
    __asm { vmovups [rbp+var_60], xmm0 }
    v39 = sub_6372B0(*(_QWORD *)(g_render_system + 3096), (__int64)a1, (__int64)&v51);
    htile_slice_size_bytes = gnm_depth_target_get_htile_slice_size_bytes(a2);
    v41 = *(unsigned int *)(a2 + 36);
    v8 = v54;
    v42 = v46 * (htile_slice_size_bytes >> 2);
    sub_10C1F00(v54, (v41 << 8) + htile_slice_size_bytes * v10, 17412, v42);
    gnm_buffer_set_resource_memory_type((__int64)v54, 109);
    gnmx_constant_update_engine_set_rw_buffer((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5764], 0, v39, v54);
    (*(void (__fastcall **)(int *, _QWORD, __int64, __int64))(*(_QWORD *)a1 + 152LL))(
      a1,
      (v42 >> 6 << 6 < v42) + (v42 >> 6),
      1,
      1);
    LOBYTE(v8) = 1;
  }
  else
  {
    __asm { vmovss  [rbp+var_94], xmm0 }
    sub_10D8960(&v49);
    sub_10D8970(&v49, 1);
    sub_10D8980(&v49, 1);
    sub_10CA000(&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], v49);
    gnm_depth_stencil_control_init(&v48);
    gnm_depth_stencil_control_set_depth_control(&v48, 1, 7);
    gnm_depth_stencil_control_set_stencil_function(&v48, 7);
    gnm_depth_stencil_control_set_depth_enable(&v48, 1u);
    gnm_depth_stencil_control_set_stencil_enable(&v48, 1u);
    gnm_draw_command_buffer_set_depth_stencil_control((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], v48);
    gnm_stencil_op_control_init(&v47);
    gnm_stencil_op_control_set_stencil_ops(&v47, 3, 3, 3);
    gnm_draw_command_buffer_set_stencil_op_control((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], v47);
    gnm_draw_command_buffer_set_stencil_separate((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], -1);
    __asm { vmovss  xmm0, [rbp+var_94] }
    gnm_draw_command_buffer_set_depth_clear_value(&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], *(double *)&_XMM0);
    gnm_draw_command_buffer_set_stencil_clear_value(&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], a3);
    LODWORD(v8) = 0;
    gnm_draw_command_buffer_set_render_target_mask((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], 0);
    orbis_render_context_draw_depth_clear((__int64)a1);
    sub_10D8960(&v49);
    sub_10D89B0(&v49, 1);
    sub_10CA000(&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], v49);
    (*(void (__fastcall **)(int *, _QWORD, _QWORD))(*(_QWORD *)a1 + 120LL))(
      a1,
      (unsigned int)a1[66412],
      (unsigned int)a1[66413]);
    orbis_build_depth_stencil_control((__int64)&v51, a1[66406], a1[66407]);
    orbis_build_stencil_control(
      v54,
      (unsigned int)a1[66407],
      *((_BYTE *)a1 + 265644),
      *((_BYTE *)a1 + 265645),
      *((_BYTE *)a1 + 265646));
    orbis_build_stencil_op_control((__int64)&v50, a1[66407]);
    gnm_draw_command_buffer_set_depth_stencil_control((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], v51);
    gnm_draw_command_buffer_set_stencil_separate((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], v54[0]);
    gnm_draw_command_buffer_set_stencil_op_control((__int64)&a1[14882 * *((_QWORD *)a1 + 33202) + 5578], v50);
  }
  return (unsigned int)v8;
}


// orbis_render_context_draw_depth_clear at 0x8EBDA0
__int64 __fastcall orbis_render_context_draw_depth_clear(__int64 a1)
{
  __int64 v3; // r14
  int v4; // eax
  int v12; // eax
  _WORD v17[2]; // [rsp+8h] [rbp-A8h] BYREF
  char v18; // [rsp+Ch] [rbp-A4h] BYREF
  int v20; // [rsp+30h] [rbp-80h] BYREF
  char v22; // [rsp+44h] [rbp-6Ch] BYREF
  int v23; // [rsp+54h] [rbp-5Ch]
  __int64 v24; // [rsp+58h] [rbp-58h]
  char v25; // [rsp+60h] [rbp-50h]
  __int16 v27; // [rsp+78h] [rbp-38h]
  __int64 v28; // [rsp+80h] [rbp-30h]

  _R15 = &v18;
  v28 = 0x6365786562696C2FLL;
  v3 = *(_QWORD *)(g_orbis_render_system + 2952);
  v17[0] = 0;
  if ( byte_19E3F50[0] == 0 )
  {
    _cxa_guard_acquire(byte_19E3F50);
    if ( v4 != 0 )
    {
      __asm { vmovups xmm0, cs:xmmword_12D1900 }
      _RAX = &unk_19E3F40;
      __asm { vmovups xmmword ptr [rax], xmm0 }
      _cxa_guard_release(byte_19E3F50);
    }
  }
  _R13 = &unk_19E3F40;
  __asm
  {
    vmovups xmm0, xmmword ptr [r13+0]
    vmovups xmmword ptr [r15], xmm0
    vxorps  xmm0, xmm0, xmm0
    vmovups [rbp+var_90], xmm0
  }
  sub_639980(v3, a1, (__int64)v17);
  if ( *(_BYTE *)(a1 + 280713) != 0 )
  {
    sub_10C5F80(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 22312, 0, 204);
    *(_BYTE *)(a1 + 280713) = 0;
  }
  gnmx_gfx_context_set_pixel_shader(
    (__int64 *)(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 23056),
    nullptr,
    a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 38112);
  __asm { vmovups xmm0, cs:xmmword_12D1910 }
  v20 = 1;
  _R14 = &v22;
  __asm { vmovups [rbp+var_7C], xmm0 }
  if ( byte_19E3F50[0] == 0 )
  {
    _cxa_guard_acquire(byte_19E3F50);
    if ( v12 != 0 )
    {
      __asm { vmovups xmm0, cs:xmmword_12D1900 }
      __asm { vmovups xmmword ptr [r13+0], xmm0 }
      _cxa_guard_release(byte_19E3F50);
    }
  }
  __asm { vmovups xmm0, xmmword ptr [r13+0] }
  __asm
  {
    vmovups xmmword ptr [r14], xmm0
    vxorps  xmm0, xmm0, xmm0
  }
  v23 = 5;
  v25 = 0;
  v24 = 0;
  __asm { vmovups [rbp+var_48], xmm0 }
  v27 = 257;
  sub_3E0C50(a1, (__int64)&v20);
  if ( *(_BYTE *)(a1 + 280713) != 1 )
  {
    sub_10C5F80(a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 22312, 1, 204);
    *(_BYTE *)(a1 + 280713) = 1;
  }
  return 0x6365786562696C2FLL;
}

