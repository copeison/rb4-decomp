/* Generated Hex-Rays evidence for Orbis geometry and vertex shader backends. */

/* 0x8E40D0 */
__int64 __fastcall orbis_geometry_shader_destruct(_QWORD *a1)
{
  *a1 = &unk_195F8B0;
  sub_642310(a1);
  return nullsub_48(a1);
}

/* 0x8E4100 */
double __fastcall orbis_geometry_shader_delete(_QWORD *a1)
{
  *a1 = &unk_195F8B0;
  sub_642310(a1);
  nullsub_48(a1);
  return sub_37BF50(a1);
}

/* 0x8E4140 */
char __fastcall orbis_geometry_shader_initialize(_QWORD *a1, __int64 a2)
{
  double v4; // xmm0_8
  double v5; // xmm0_8
  int v6; // eax
  __int64 v7; // rdi
  __int64 v8; // rsi
  __int64 v9; // rcx
  __int64 v10; // rax
  __int64 v11; // rax
  unsigned __int64 v12; // rdx
  unsigned __int64 v13; // rsi
  __int64 v14; // rcx
  _BYTE v16[8]; // [rsp+0h] [rbp-70h] BYREF
  __int64 v17; // [rsp+8h] [rbp-68h]
  unsigned int v18; // [rsp+10h] [rbp-60h]
  _QWORD v19[2]; // [rsp+18h] [rbp-58h] BYREF
  unsigned int v20; // [rsp+28h] [rbp-48h]
  _QWORD v21[8]; // [rsp+30h] [rbp-40h] BYREF

  v21[2] = 0x6365786562696C2FLL;
  v4 = sub_11B2EE0(v21);
  sub_37AA30(v19, 1, 1, v4);
  v5 = sub_11B2F30(v21, a2);
  sub_37AAF0(v19, v5);
  sub_10EF310(v19, v16, v21[0]);
  if ( byte_1ADF190 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADF190);
    if ( v6 != 0 )
    {
      qword_1ADF188 = sub_37BA70("gpu");
      _cxa_guard_release(&byte_1ADF190);
    }
  }
  sub_37A920(qword_1ADF188);
  a1[7] = sub_37AE70(v20, "GShader", 256);
  a1[8] = sub_37AE70(v18, "GShader", 256);
  sub_37A9B0(v7, v8);
  v9 = HIBYTE(*(_DWORD *)v19[0]);
  a1[6] = sub_37AE70(
            ((4
            * (*(unsigned __int8 *)(v19[0] + 4 * v9 + 59) + (unsigned __int16)*(unsigned __int8 *)(v19[0] + 4 * v9 + 92))
            + 2 * *(unsigned __int8 *)(v19[0] + 4 * v9 + 93)
            + 43)
           & 0xFFCu)
          + 4 * (_DWORD)v9
          + 56,
            "GShader",
            4);
  memcpy(a1[7], v19[1], v20);
  memcpy(a1[8], v17, v18);
  v10 = HIBYTE(*(_DWORD *)v19[0]);
  memcpy(
    a1[6],
    v19[0],
    ((4 * (*(unsigned __int8 *)(v19[0] + 4 * v10 + 59) + (unsigned __int16)*(unsigned __int8 *)(v19[0] + 4 * v10 + 92))
    + 2 * *(unsigned __int8 *)(v19[0] + 4 * v10 + 93)
    + 43)
   & 0xFFCu)
  + 4 * (_DWORD)v10
  + 56);
  v11 = a1[6];
  a1[5] = v11;
  v12 = a1[8];
  v13 = a1[7];
  *(_DWORD *)(v11 + 8) = v13 >> 8;
  *(_DWORD *)(v11 + 12) = v13 >> 40;
  v14 = *(unsigned __int8 *)(v11 + 3);
  *(_DWORD *)(v11 + 4 * v14 + 64) = v12 >> 8;
  *(_DWORD *)(v11 + 4 * v14 + 68) = v12 >> 40;
  sub_11B2F00(v21);
  nullsub_240(v21);
  return 1;
}

/* 0x8E4330 */
None

/* 0x8E4350 */
void __fastcall orbis_geometry_shader_release_backend(_QWORD *a1)
{
  orbis_defer_allocation_release(g_orbis_render_system, a1[6]);
  orbis_defer_allocation_release(g_orbis_render_system, a1[7]);
  orbis_defer_allocation_release(g_orbis_render_system, a1[8]);
  a1[5] = 0;
}

/* 0x8E43A0 */
__int64 orbis_geometry_shader_stage()
{
  return 3;
}

/* 0x8E4720 */
__int64 __fastcall orbis_vertex_shader_destruct(_QWORD *a1)
{
  *a1 = &unk_195F930;
  sub_642310(a1);
  return nullsub_48(a1);
}

/* 0x8E4750 */
double __fastcall orbis_vertex_shader_delete(_QWORD *a1)
{
  *a1 = &unk_195F930;
  sub_642310(a1);
  nullsub_48(a1);
  return sub_37BF50(a1);
}

/* 0x8E4790 */
char __fastcall orbis_vertex_shader_initialize(_QWORD *a1, __int64 a2)
{
  double v4; // xmm0_8
  double v5; // xmm0_8
  int v6; // eax
  __int64 v7; // rdi
  __int64 v8; // rsi
  __int64 v13; // rdx
  unsigned __int64 v14; // rsi
  unsigned __int64 v15; // rax
  __int64 v16; // rcx
  __int64 v17; // rax
  __int64 v18; // r9
  __int64 v19; // r8
  __int64 v20; // rsi
  __int64 v26; // rdi
  bool v34; // zf
  __int64 v60; // rdi
  _BYTE *v61; // rdx
  unsigned __int8 v62; // cl
  unsigned __int64 v63; // r15
  __int64 v64; // rdx
  __int64 v65; // rsi
  __int64 v66; // rbx
  double v67; // xmm0_8
  unsigned __int64 v68; // rcx
  unsigned __int64 v70; // rax
  unsigned __int64 v72; // rdi
  int v73; // eax
  unsigned int v74; // eax
  unsigned int v75; // eax
  __int64 v76; // rdi
  __int64 v77; // rsi
  __int64 v78; // rsi
  _BYTE v80[8]; // [rsp+0h] [rbp-60h] BYREF
  _QWORD v81[2]; // [rsp+8h] [rbp-58h] BYREF
  unsigned int v82; // [rsp+18h] [rbp-48h]
  _QWORD v83[8]; // [rsp+20h] [rbp-40h] BYREF

  v83[2] = 0x6365786562696C2FLL;
  v4 = sub_11B2EE0(v83);
  sub_37AA30(v81, 1, 1, v4);
  v5 = sub_11B2F30(v83, a2);
  sub_37AAF0(v81, v5);
  sub_10EF2E0(v81, v83[0]);
  if ( byte_1ADF1D0 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADF1D0);
    if ( v6 != 0 )
    {
      qword_1ADF1C8 = sub_37BA70(byte_12D1800);
      _cxa_guard_release(&byte_1ADF1D0);
    }
  }
  sub_37A920(qword_1ADF1C8);
  a1[9] = sub_37AE70(v82, "VShader", 256);
  sub_37A9B0(v7, v8);
  a1[7] = sub_37AE70(
            (4 * (*(unsigned __int8 *)(v81[0] + 3LL) + (unsigned __int16)*(unsigned __int8 *)(v81[0] + 36LL))
           + 2 * *(unsigned __int8 *)(v81[0] + 37LL)
           + 43)
          & 0xFFC,
            "VShader",
            4);
  a1[8] = sub_37AE70(
            4 * (*(unsigned __int8 *)(v81[0] + 24LL) + (unsigned __int64)*(unsigned __int8 *)(v81[0] + 3LL)) + 32,
            "VShader",
            4);
  memcpy(a1[9], v81[1], v82);
  memcpy(
    a1[7],
    v81[0],
    (4 * (*(unsigned __int8 *)(v81[0] + 3LL) + (unsigned __int16)*(unsigned __int8 *)(v81[0] + 36LL))
   + 2 * *(unsigned __int8 *)(v81[0] + 37LL)
   + 43)
  & 0xFFC);
  memcpy(
    a1[8],
    v81[0],
    4 * (*(unsigned __int8 *)(v81[0] + 24LL) + (unsigned __int64)*(unsigned __int8 *)(v81[0] + 3LL)) + 32);
  v13 = a1[7];
  a1[5] = v13;
  v14 = a1[9];
  v15 = v14 >> 40;
  v14 >>= 8;
  *(_DWORD *)(v13 + 8) = v14;
  *(_DWORD *)(v13 + 12) = v15;
  v16 = a1[8];
  a1[6] = v16;
  *(_DWORD *)(v16 + 8) = v14;
  *(_DWORD *)(v16 + 12) = v15;
  v17 = *(unsigned __int8 *)(v13 + 36);
  if ( *(_BYTE *)(v13 + 36) == 0 )
  {
    _R13 = 0;
    goto LABEL_18;
  }
  v18 = *(unsigned __int8 *)(v13 + 3);
  if ( (unsigned __int8)v17 >= 8u )
  {
    v19 = 8;
    if ( (v17 & 7) != 0 )
      v19 = v17 & 7;
    v20 = v17 - v19;
    if ( v17 != v19 )
    {
      __asm
      {
        vmovdqu xmm8, cs:xmmword_12D1770
        vmovdqu xmm9, cs:xmmword_12D1780
        vmovdqu xmm10, cs:xmmword_12D1790
      }
      _RBX = v13 + 4 * v18 + 64;
      __asm { vpxor   xmm0, xmm0, xmm0 }
      v26 = v17 - v19;
      __asm
      {
        vpxor   xmm4, xmm4, xmm4
        vpxor   xmm5, xmm5, xmm5
        vpxor   xmm6, xmm6, xmm6
      }
      do
      {
        __asm
        {
          vmovq   xmm7, qword ptr [rbx-18h]
          vmovq   xmm1, qword ptr [rbx-10h]
          vmovq   xmm2, qword ptr [rbx-8]
          vmovq   xmm3, qword ptr [rbx]
        }
        _RBX += 32;
        v34 = v26 == 8;
        v26 -= 8;
        __asm
        {
          vpshufb xmm7, xmm7, xmm8
          vpshufb xmm1, xmm1, xmm8
          vpshufb xmm2, xmm2, xmm8
          vpshufb xmm3, xmm3, xmm8
          vpxor   xmm7, xmm7, xmm9
          vpxor   xmm1, xmm1, xmm9
          vpxor   xmm2, xmm2, xmm9
          vpxor   xmm3, xmm3, xmm9
          vpcmpgtq xmm7, xmm10, xmm7
          vpcmpgtq xmm1, xmm10, xmm1
          vpcmpgtq xmm2, xmm10, xmm2
          vpcmpgtq xmm3, xmm10, xmm3
          vpsrlq  xmm7, xmm7, 3Fh ; '?'
          vpsrlq  xmm1, xmm1, 3Fh ; '?'
          vpsrlq  xmm2, xmm2, 3Fh ; '?'
          vpsrlq  xmm3, xmm3, 3Fh ; '?'
          vpaddq  xmm0, xmm7, xmm0
          vpaddq  xmm4, xmm1, xmm4
          vpaddq  xmm5, xmm2, xmm5
          vpaddq  xmm6, xmm3, xmm6
        }
      }
      while ( !v34 );
      __asm { vpaddq  xmm0, xmm4, xmm0 }
      __asm
      {
        vpaddq  xmm0, xmm5, xmm0
        vpaddq  xmm0, xmm6, xmm0
        vpshufd xmm1, xmm0, 4Eh ; 'N'
        vpaddq  xmm0, xmm0, xmm1
        vmovq   r13, xmm0
      }
      if ( v19 != 0 )
        goto LABEL_14;
LABEL_18:
      v62 = *(_BYTE *)(v16 + 24);
      if ( (unsigned __int8)v17 >= v62 )
        v62 = v17;
      v63 = v62;
      sub_37AA30(v80, 1, 1, *(double *)&_XMM0);
      v66 = sub_37BF60(4 * v63, v65, v64);
      sub_37AAF0(v80, v67);
      if ( _R13 != 0 )
        memset(v66, 0, 4 * _R13);
      v68 = v63 - _R13;
      if ( v63 > _R13 )
      {
        if ( v68 > 0x1F && (v68 & 0xFFFFFFFFFFFFFFE0LL) != 0 )
        {
          __asm { vmovdqu ymm0, cs:ymmword_12D17E0 }
          v70 = _R13 + (v68 & 0xFFFFFFFFFFFFFFE0LL);
          _RSI = v66 + 4 * _R13 + 96;
          v72 = v68 & 0xFFFFFFFFFFFFFFE0LL;
          do
          {
            __asm
            {
              vmovdqu ymmword ptr [rsi-60h], ymm0
              vmovdqu ymmword ptr [rsi-40h], ymm0
              vmovdqu ymmword ptr [rsi-20h], ymm0
              vmovdqu ymmword ptr [rsi], ymm0
            }
            _RSI += 128;
            v72 -= 32LL;
          }
          while ( v72 != 0 );
          if ( v68 == (v68 & 0xFFFFFFFFFFFFFFE0LL) )
            goto LABEL_31;
        }
        else
        {
          v70 = _R13;
        }
        do
          *(_DWORD *)(v66 + 4 * v70++) = 1;
        while ( v70 < v63 );
      }
LABEL_31:
      if ( byte_1ADF1E0 == 0 )
      {
        _cxa_guard_acquire(&byte_1ADF1E0);
        if ( v73 != 0 )
        {
          qword_1ADF1D8 = sub_37BA70(byte_12D1800);
          _cxa_guard_release(&byte_1ADF1E0);
        }
      }
      sub_37A920(qword_1ADF1D8);
      v74 = sub_10E9750(a1[5]);
      a1[11] = sub_37AE70(v74, "VShader", 4);
      v75 = sub_10E9F50(a1[6]);
      a1[12] = sub_37AE70(v75, "VShader", 4);
      sub_37A9B0(v76, v77);
      sub_10E9990(a1[11], a1 + 10, a1[5], v66, (unsigned int)v63);
      sub_10EA190(a1[12], a1 + 10, a1[6], v66, (unsigned int)v63);
      sub_37BF70(v66, v78);
      sub_11B2F00(v83);
      nullsub_240(v83);
      return 1;
    }
  }
  v20 = 0;
  _R13 = 0;
LABEL_14:
  v60 = v17 - v20;
  v61 = (_BYTE *)(v13 + 4 * (v20 + v18) + 40);
  do
  {
    _R13 += *v61 < 8u;
    v61 += 4;
    --v60;
  }
  while ( v60 != 0 );
  goto LABEL_18;
}

/* 0x8E4D80 */
void __fastcall orbis_vertex_shader_bind(__int64 a1, __int64 a2)
{
  __int64 v4; // rax
  __int64 v5; // r14
  __int64 v6; // rdx
  __int64 v7; // r12
  __int64 v8; // rax
  __int64 *v9; // r15
  __int64 v10; // r13
  __int64 v11; // rax
  __int64 v12; // r14
  __int64 v13; // rcx
  __int64 v14; // r12
  __int64 v15; // r15
  __int64 v16; // r13
  __int64 v17; // rdi
  unsigned int v18; // ebx
  unsigned int v19; // [rsp+0h] [rbp-2Ch]

  v4 = *(_QWORD *)(a2 + 265616);
  if ( (*(_BYTE *)(a2 + 18784) & 8) != 0 )
  {
    gnmx_gfx_context_set_vertex_shader(
      (__int64 *)(a2 + 59528 * v4 + 23056),
      nullptr,
      *(unsigned int *)(a1 + 80),
      0,
      a2 + 59528 * v4 + 44296);
    v11 = 59528LL * *(_QWORD *)(a2 + 265616);
    v12 = *(_QWORD *)(a1 + 48);
    v13 = *(unsigned int *)(a1 + 80);
    v14 = *(_QWORD *)(a1 + 96);
    v15 = a2 + v11 + 23056;
    v16 = a2 + v11 + 56664;
    if ( v12 != 0 && *(_QWORD *)((char *)&loc_13E38 + a2 + v11) != v12 )
    {
      v17 = a2 + v11 + 56664;
      v18 = v13;
      sub_10E7BA0(v17, v12 + 32, *(unsigned __int8 *)(v12 + 3));
      v13 = v18;
    }
    sub_10E6C30(v15, v12, 0, v13, v14, v16);
  }
  else
  {
    v5 = *(_QWORD *)(a1 + 40);
    v6 = *(unsigned int *)(a1 + 80);
    v7 = *(_QWORD *)(a1 + 88);
    v8 = 59528 * v4;
    v9 = (__int64 *)(a2 + v8 + 23056);
    v10 = a2 + v8 + 44296;
    if ( v5 != 0 && *(_QWORD *)(a2 + v8 + 81424) != v5 )
    {
      v19 = *(_DWORD *)(a1 + 80);
      sub_10E7BA0(a2 + v8 + 44296, v5 + 40, *(unsigned __int8 *)(v5 + 3));
      v6 = v19;
    }
    gnmx_gfx_context_set_vertex_shader(v9, (_DWORD *)v5, v6, v7, v10);
    sub_10E6C30(
      a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056,
      0,
      0,
      0,
      0,
      a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 56664);
  }
}

/* 0x8E4ED0 */
void __fastcall orbis_vertex_shader_release_backend(__int64 *a1)
{
  _RBX = a1;
  orbis_defer_allocation_release(g_orbis_render_system, a1[11]);
  orbis_defer_allocation_release(g_orbis_render_system, _RBX[12]);
  orbis_defer_allocation_release(g_orbis_render_system, _RBX[7]);
  orbis_defer_allocation_release(g_orbis_render_system, _RBX[8]);
  *(double *)&_XMM0 = orbis_defer_allocation_release(g_orbis_render_system, _RBX[9]);
  __asm
  {
    vxorps  xmm0, xmm0, xmm0
    vmovups xmmword ptr [rbx+28h], xmm0
  }
}

/* 0x8E4F30 */
__int64 orbis_vertex_shader_stage()
{
  return 0;
}
