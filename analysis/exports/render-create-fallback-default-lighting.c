__int64 __fastcall render_create_fallback_default_lighting(_QWORD *a1, __int64 a2)
{
  __int64 v4; // rax
  __int64 v5; // rdi
  __int64 v6; // rdx
  _QWORD *v7; // rax
  __int64 v8; // rbx
  __int64 v9; // r15
  __int64 v10; // r8
  __int64 v11; // r9
  __int64 v12; // rdx
  bool v13; // zf
  __int64 v14; // rdx
  _QWORD *v15; // rax
  char *v66; // rbx
  int v67; // r12d
  char *v68; // rsi
  __int64 v69; // r13
  __int64 v70; // rax
  __int64 v71; // r15
  char *v72; // rbx
  __int64 v73; // rbx
  __int64 v74; // rsi
  __m256 v76; // [rsp+0h] [rbp-70h] BYREF
  __int128 v77; // [rsp+20h] [rbp-50h]
  _QWORD v78[7]; // [rsp+38h] [rbp-38h] BYREF

  v78[1] = 0x6365786562696C2FLL;
  v4 = **(_QWORD **)(*(_QWORD *)(a2 + 176) + 8LL);
  if ( *(_DWORD *)(v4 + 64) != 0 )
  {
    v5 = 0;
    v6 = 24LL * *(unsigned int *)(v4 + 64);
    v7 = (_QWORD *)(*(_QWORD *)(v4 + 56) + 8LL);
    while ( *v7 != unk_1A722A8 )
    {
      v7 += 3;
      v6 -= 24;
      if ( v6 == 0 )
        goto LABEL_8;
    }
    v5 = *(v7 - 1);
  }
  else
  {
    v5 = 0;
  }
LABEL_8:
  a1[60] = sub_408F60(v5);
  v8 = 0;
  v9 = sub_F0A10(a2, 0, 0);
  sub_256FD0(v78, "default_directional_light");
  sub_1160F0(v9, v78[0]);
  *(_DWORD *)(sub_117700((_BYTE *)v9, unk_1A87640, 0) + 88) = 0x40000000;
  v12 = *(unsigned int *)(v9 + 64);
  v13 = v12 == 0;
  if ( *(_DWORD *)(v9 + 64) != 0 )
  {
    v8 = 0;
    v14 = 24 * v12;
    v15 = (_QWORD *)(*(_QWORD *)(v9 + 56) + 8LL);
    while ( 1 )
    {
      v13 = *v15 == unk_19E46E8;
      if ( *v15 == unk_19E46E8 )
        break;
      v15 += 3;
      v13 = v14 == 24;
      v14 -= 24;
      if ( v14 == 0 )
        goto LABEL_14;
    }
    v8 = *(v15 - 1);
  }
LABEL_14:
  _RAX = &unk_19E666C;
  _RSI = &unk_19B0358;
  _ECX = HIDWORD(qword_1AB007C);
  _EDX = dword_1AB0084;
  __asm
  {
    vmovups ymm1, ymmword ptr [rax+10h]
    vmovss  xmm4, dword ptr [rsi+8]
    vmovss  xmm6, dword ptr [rsi+4]
    vmovss  xmm5, dword ptr [rsi]
    vmovd   xmm8, ecx
    vmovups ymm0, ymmword ptr [rax]
    vmovups [rbp+var_70+10h], ymm1
  }
  v77 = *(_OWORD *)&_RT0.m256i_u64[2];
  __asm
  {
    vmovd   xmm1, edx
    vmulss  xmm2, xmm4, xmm8
    vmovups [rbp+var_70], ymm0
  }
  *(_QWORD *)&v76.m256_f32[6] = _RT0.m256i_i64[3];
  *(_QWORD *)&v76.m256_f32[3] = qword_1AB007C;
  LODWORD(v76.m256_f32[5]) = dword_1AB0084;
  __asm
  {
    vmulss  xmm3, xmm6, xmm1
    vmulss  xmm7, xmm5, xmm1
    vmulss  xmm5, xmm5, xmm8
    vsubss  xmm9, xmm2, xmm3
    vmovd   xmm2, edi
    vmulss  xmm4, xmm2, xmm4
    vmulss  xmm6, xmm2, xmm6
    vsubss  xmm4, xmm7, xmm4
    vsubss  xmm5, xmm6, xmm5
    vmulss  xmm6, xmm9, xmm9
    vmovss  dword ptr [rbp+var_70], xmm9
    vmulss  xmm7, xmm4, xmm4
    vmovss  dword ptr [rbp+var_70+4], xmm4
    vmovss  dword ptr [rbp+var_70+8], xmm5
    vaddss  xmm6, xmm7, xmm6
    vmulss  xmm7, xmm5, xmm5
    vaddss  xmm7, xmm6, xmm7
    vrsqrtss xmm6, xmm7, xmm7
    vmulss  xmm0, xmm7, xmm6
    vmulss  xmm3, xmm0, cs:dword_12B6C40
    vmulss  xmm0, xmm0, xmm6
    vaddss  xmm0, xmm0, cs:dword_12B6C44
    vxorps  xmm6, xmm6, xmm6
    vmulss  xmm0, xmm3, xmm0
    vcmpeqss xmm3, xmm7, xmm6
    vandnps xmm7, xmm3, xmm0
    vucomiss xmm7, xmm6
  }
  if ( !v13 )
  {
    __asm
    {
      vmovss  xmm0, cs:dword_12B6C48
      vdivss  xmm6, xmm0, xmm7
    }
  }
  __asm
  {
    vmulss  xmm0, xmm6, xmm9
    vmulss  xmm3, xmm6, xmm4
    vmulss  xmm4, xmm6, xmm5
  }
  __asm
  {
    vmovss  dword ptr [rbp+var_70], xmm0
    vmovss  dword ptr [rbp+var_70+4], xmm3
    vmulss  xmm5, xmm1, xmm3
    vmulss  xmm6, xmm8, xmm4
    vmovss  dword ptr [rbp+var_70+8], xmm4
    vmulss  xmm4, xmm2, xmm4
    vmulss  xmm1, xmm0, xmm1
    vmulss  xmm0, xmm0, xmm8
    vmulss  xmm2, xmm2, xmm3
    vsubss  xmm5, xmm5, xmm6
    vsubss  xmm1, xmm4, xmm1
    vsubss  xmm0, xmm0, xmm2
    vmovss  dword ptr [rbp+var_70+18h], xmm5
    vmovss  dword ptr [rbp+var_70+1Ch], xmm1
    vmovss  dword ptr [rbp+var_50], xmm0
  }
  ((void (__fastcall *)(__int64, __m256 *, __int64, _QWORD, __int64, __int64))sub_127730)(
    v8 + 24,
    &v76,
    1,
    HIDWORD(qword_1AB007C),
    v10,
    v11);
  *(_QWORD *)(v8 + 48) = *(_QWORD *)((char *)&v77 + 4);
  *(_DWORD *)(v8 + 56) = HIDWORD(v77);
  *(_BYTE *)(v8 + 72) |= 1u;
  v66 = (char *)a1[63];
  v67 = *(_DWORD *)(v9 + 88);
  if ( (unsigned __int64)v66 >= a1[64] )
  {
    v68 = (char *)a1[62];
    v69 = 1;
    if ( v66 != v68 )
      v69 = (v66 - v68) >> 1;
    if ( v69 != 0 )
    {
      v70 = sub_252CF0(a1 + 65, 4 * v69, 0);
      v68 = (char *)a1[62];
      v66 = (char *)a1[63];
      v71 = v70;
    }
    else
    {
      v71 = 0;
    }
    v72 = (char *)(v66 - v68);
    memmove(v71, v68, v72);
    *(_DWORD *)&v72[v71] = v67;
    v73 = (__int64)&v72[v71 + 4];
    v74 = a1[62];
    if ( v74 != 0 )
      sub_252D30(a1 + 65, v74, a1[64] - v74);
    a1[62] = v71;
    a1[63] = v73;
    a1[64] = v71 + 4 * v69;
  }
  else
  {
    a1[63] = v66 + 4;
    *(_DWORD *)v66 = v67;
  }
  render_apply_default_lighting_mode((__int64)a1);
  return 0x6365786562696C2FLL;
}
