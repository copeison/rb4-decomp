__int64 __fastcall fmod_buffered_stream_generator_render_callback(__int64 a1, unsigned __int64 a2)
{
  _QWORD *v7; // r14
  signed __int64 v8; // rsi
  double v10; // xmm0_8
  __int64 v15; // rbx
  int v16; // eax
  __int64 v17; // rdx
  bool v18; // zf
  __int64 v21; // r12
  __int64 v22; // rdx
  unsigned __int64 v23; // rbx
  __int64 v24; // rdx
  unsigned __int64 v25; // rdi
  __int64 v26; // rbx
  __int64 v27; // rbx
  char v34; // cl
  bool v36; // cc
  int v38; // ecx
  int v41; // edi
  int v49; // ecx
  int v52; // edi
  int v57; // eax
  __int64 v58; // rax
  _QWORD *v59; // rdi
  __int64 v66; // r15
  __int64 v69; // r10
  __int16 *v74; // r14
  int v75; // edi
  __int64 v201; // rbx
  bool v216; // zf
  int v220; // ecx
  int v222; // edi
  __int16 *v227; // rdi
  __int16 *v228; // rcx
  __int16 *v229; // rsi
  __int16 *v230; // rdx
  bool v245; // cf
  __int64 v246; // rcx
  int v248; // ecx
  __int64 v249; // rcx
  char v251; // r10
  signed __int64 v252; // rdi
  unsigned __int64 v253; // r14
  unsigned __int64 v254; // r11
  unsigned __int64 v255; // r9
  __int64 v256; // r12
  int v257; // eax
  __int64 v258; // rbx
  char v259; // r15
  __int64 v260; // r10
  unsigned __int64 v262; // rbx
  __int64 v269; // rbx
  _QWORD *v270; // [rsp+10h] [rbp-80h]
  char v271; // [rsp+18h] [rbp-78h]
  unsigned __int64 v272; // [rsp+28h] [rbp-68h]
  unsigned __int64 v273; // [rsp+30h] [rbp-60h]
  int v274; // [rsp+38h] [rbp-58h]
  int v275; // [rsp+3Ch] [rbp-54h]
  unsigned __int64 v278; // [rsp+48h] [rbp-48h]
  int v279; // [rsp+54h] [rbp-3Ch]
  unsigned __int64 v280; // [rsp+58h] [rbp-38h]
  char v281; // [rsp+67h] [rbp-29h]

  _R13 = a1;
  v7 = (_QWORD *)(a1 + 80);
  (*(void (__fastcall **)(__int64))(*(_QWORD *)(a1 + 80) + 40LL))(a1 + 80);
  if ( *(_BYTE *)(_R13 + 492) != 0 )
  {
    v15 = *(_QWORD *)(_R13 + 280);
    if ( *(_DWORD *)(v15 + 192LL * *(int *)(_R13 + 496) - 128) != 3 )
    {
      if ( *(_DWORD *)(a2 + 16) != 0 && *(_BYTE *)(a2 + 100) == 0 )
      {
        if ( *(int *)(a2 + 104) > 0 )
        {
          v27 = 0;
          do
            memset(*(_QWORD *)(a2 + 8 * v27++ + 24), 0, 4LL * *(int *)(a2 + 96));
          while ( v27 < *(int *)(a2 + 104) );
        }
        goto LABEL_47;
      }
      v23 = *(_QWORD *)(a2 + 24);
      v24 = (int)sub_D3D30(a2 + 88, v10);
      v25 = v23;
      goto LABEL_46;
    }
    *(_BYTE *)(_R13 + 492) = 0;
  }
  else
  {
    v15 = *(_QWORD *)(_R13 + 280);
  }
  v16 = *(_DWORD *)(_R13 + 488);
  v17 = 192LL * v16;
  if ( (*(_DWORD *)(v15 + v17 + 164) | 0x10) != 0x10 )
  {
    if ( *(_DWORD *)(a2 + 16) != 0 && *(_BYTE *)(a2 + 100) == 0 )
    {
      if ( *(int *)(a2 + 104) > 0 )
      {
        v26 = 0;
        do
          memset(*(_QWORD *)(a2 + 8 * v26++ + 24), 0, 4LL * *(int *)(a2 + 96));
        while ( v26 < *(int *)(a2 + 104) );
      }
    }
    else
    {
      v21 = *(_QWORD *)(a2 + 24);
      v22 = (int)sub_D3D30(a2 + 88, v10);
      memset(v21, 0, 4 * v22);
    }
LABEL_12:
    *(_BYTE *)(a2 + 124) = 1;
    if ( (unsigned int)(*(_DWORD *)(_R13 + 28) - 5) >= 2 )
    {
      *(_DWORD *)(_R13 + 28) = 6;
      (*(void (__fastcall **)(_QWORD))(**(_QWORD **)(_R13 + 312) + 16LL))(*(_QWORD *)(_R13 + 312));
    }
    goto LABEL_48;
  }
  v18 = *(_BYTE *)(_R13 + 524) == 0;
  if ( *(_BYTE *)(_R13 + 524) != 0 )
  {
    __asm
    {
      vmovss  xmm1, dword ptr [r13+208h]
      vmovss  xmm2, cs:dword_125CD5C
      vmovss  xmm0, dword ptr [r13+204h]
    }
    *(_BYTE *)(_R13 + 524) = 0;
    __asm { vucomiss xmm2, xmm1 }
    if ( v18 )
    {
      __asm { vucomiss xmm0, cs:dword_125CD5C }
      *(_QWORD *)(_R13 + 508) = -1;
    }
    else
    {
      __asm { vucomiss xmm1, cs:dword_125CD5C }
      __asm
      {
        vmulss  xmm2, xmm1, cs:dword_125CD60
        vmovss  xmm1, dword ptr [r13+1D4h]
        vmulss  xmm2, xmm2, xmm1
        vcvttss2si ecx, xmm2
        vmulss  xmm0, xmm0, cs:dword_125CD60
      }
      *(_DWORD *)(_R13 + 512) = _ECX;
      __asm
      {
        vmulss  xmm0, xmm0, xmm1
        vcvttss2si ecx, xmm0
      }
      *(_DWORD *)(_R13 + 508) = _ECX;
    }
  }
  v280 = *(_QWORD *)(a2 + 24);
  v34 = *(_BYTE *)(_R13 + 525);
  _R9 = *(_QWORD *)(a2 + 32);
  v36 = v34 == 0;
  v281 = v34;
  if ( v34 != 0 )
  {
    __asm { vmovss  xmm0, dword ptr [r13+1E4h] }
    v38 = *(_DWORD *)(_R13 + 528);
    __asm { vmulss  xmm0, xmm0, cs:dword_125CD60 }
    __asm
    {
      vmulss  xmm0, xmm0, dword ptr [r13+1D4h]
      vcvttss2si r8d, xmm0
    }
    v8 = (unsigned int)(v38 - _R8);
    v41 = _R8 - v38;
    if ( (int)_R8 - v38 < 1 )
      v41 = v38 - _R8;
    if ( v41 <= 0 )
      goto LABEL_35;
    __asm
    {
      vmovss  xmm0, dword ptr [r13+21Ch]
      vxorps  xmm1, xmm1, xmm1
      vucomiss xmm0, xmm1
    }
    __asm { vucomiss xmm0, xmm1 }
    if ( v38 < (int)_R8 )
      goto LABEL_35;
    __asm
    {
      vmovsd  xmm1, qword ptr [r13+0E8h]
      vcvtsd2ss xmm1, xmm1, xmm1
      vdivss  xmm0, xmm0, xmm1
      vxorps  xmm1, xmm1, xmm1
      vucomiss xmm0, xmm1
    }
    if ( v38 <= (unsigned int)_R8 )
      goto LABEL_35;
    __asm
    {
      vcvtsi2ss xmm0, xmm3, dword ptr [rbx+rdx+0A0h]
      vucomiss xmm0, dword ptr [r13+1F4h]
    }
    v36 = v281 == 0;
    if ( v281 != 0 )
    {
LABEL_35:
      __asm { vmovss  xmm0, dword ptr [r13+1E4h] }
      v49 = *(_DWORD *)(_R13 + 528);
      __asm { vmulss  xmm0, xmm0, cs:dword_125CD60 }
      __asm
      {
        vmulss  xmm0, xmm0, dword ptr [r13+1D4h]
        vcvttss2si r8d, xmm0
      }
      v8 = (unsigned int)(v49 - _R8);
      v52 = _R8 - v49;
      if ( (int)_R8 - v49 < 1 )
        v52 = v49 - _R8;
      if ( v52 <= 0 )
        goto LABEL_42;
      __asm
      {
        vmovss  xmm0, dword ptr [r13+21Ch]
        vxorps  xmm1, xmm1, xmm1
        vucomiss xmm0, xmm1
      }
      __asm { vucomiss xmm0, xmm1 }
      v36 = v49 <= (unsigned int)_R8;
      if ( v49 < (int)_R8 )
      {
LABEL_42:
        if ( *(int *)(v15 + v17 + 136) <= 0 )
        {
          __asm { vmovss  xmm0, dword ptr [r13+1F4h] }
          __asm
          {
            vcvtsi2ss xmm1, xmm3, ecx
            vucomiss xmm0, xmm1
          }
          if ( *(_DWORD *)(a2 + 16) != 0 && *(_BYTE *)(a2 + 100) == 0 )
          {
            if ( *(int *)(a2 + 104) > 0 )
            {
              memset(v280, 0, 4LL * *(int *)(a2 + 96));
              if ( *(int *)(a2 + 104) >= 2 )
              {
                v258 = 4;
                do
                {
                  memset(*(_QWORD *)(a2 + 8 * v258), 0, 4LL * *(int *)(a2 + 96));
                  v36 = v258 - 2 < *(int *)(a2 + 104);
                  ++v258;
                }
                while ( v36 );
              }
            }
            goto LABEL_47;
          }
          v57 = sub_D3D30(a2 + 88, *(double *)&_XMM0);
          v25 = v280;
          v24 = v57;
LABEL_46:
          memset(v25, 0, 4 * v24);
LABEL_47:
          *(_BYTE *)(a2 + 124) = 1;
LABEL_48:
          v58 = *v7;
          v59 = v7;
          return (*(__int64 (__fastcall **)(_QWORD *))(v58 + 56))(v59);
        }
        goto LABEL_50;
      }
      __asm
      {
        vmovsd  xmm1, qword ptr [r13+0E8h]
        vcvtsd2ss xmm1, xmm1, xmm1
        vdivss  xmm0, xmm0, xmm1
      }
    }
    else
    {
      __asm { vmovss  xmm0, dword ptr [r13+1D8h] }
    }
  }
  else
  {
    __asm
    {
      vmovss  xmm0, dword ptr [r13+1D8h]
      vxorps  xmm1, xmm1, xmm1
      vucomiss xmm1, xmm0
    }
  }
  __asm
  {
    vxorps  xmm1, xmm1, xmm1
    vucomiss xmm0, xmm1
  }
  if ( v36 )
    goto LABEL_42;
LABEL_50:
  _RCX = *(unsigned int *)(a2 + 96);
  v270 = v7;
  v273 = a2;
  if ( (int)_RCX <= 0 )
  {
    _RDX = 0;
    v251 = 0;
    v271 = 0;
    goto LABEL_115;
  }
  __asm
  {
    vmovss  xmm1, cs:dword_125CDB4
    vmovss  xmm3, cs:dword_125CD5C
    vmovss  xmm2, cs:dword_125CD60
    vmovss  xmm13, cs:dword_125CD70
    vmovss  xmm14, cs:dword_125CDA8
  }
  v66 = v15;
  _R12 = 0;
  __asm { vxorps  xmm6, xmm6, xmm6 }
  _R8 = 0;
  v279 = *(_DWORD *)(v15 + v17 + 136);
  v274 = v279 + *(_DWORD *)(v15 + v17 + 160);
  v275 = *(_DWORD *)(_R13 + 464);
  v69 = *(_QWORD *)(v15 + v17 + 144);
  v278 = *(_QWORD *)(_R13 + 344);
  _RDX = v278 + 4LL * *(int *)(_R13 + 416);
  __asm { vmovss  xmm4, dword ptr [r13+1F4h] }
  v272 = _RDX;
  __asm
  {
    vroundss xmm0, xmm4, xmm4, 9
    vcvttss2si r11d, xmm0
  }
  v74 = (__int16 *)(v69 + 4LL * _R11D);
  v271 = 0;
  while ( 1 )
  {
    __asm { vcvtsi2ss xmm5, xmm9, r11d }
    __asm { vsubss  xmm15, xmm4, xmm5 }
    if ( v281 != 0 )
    {
      __asm { vmulss  xmm4, xmm2, dword ptr [r13+1E4h] }
      v220 = *(_DWORD *)(_R13 + 528);
      __asm { vmulss  xmm4, xmm4, dword ptr [r13+1D4h] }
      __asm { vcvttss2si edx, xmm4 }
      v8 = (unsigned int)(v220 - _RDX);
      v222 = _RDX - v220;
      if ( (int)_RDX - v220 < 1 )
        v222 = v220 - _RDX;
      if ( v222 <= 0 )
        goto LABEL_68;
      __asm
      {
        vmovss  xmm4, dword ptr [r13+21Ch]
        vucomiss xmm4, xmm6
      }
      __asm { vucomiss xmm4, xmm6 }
      if ( v220 < (int)_RDX )
        goto LABEL_68;
      __asm
      {
        vmovsd  xmm5, qword ptr [r13+0E8h]
        vcvtsd2ss xmm5, xmm5, xmm5
        vdivss  xmm4, xmm4, xmm5
        vucomiss xmm4, xmm6
      }
      if ( v220 == (_DWORD)_RDX )
      {
LABEL_68:
        _R11 = v280;
        v201 = v66;
        *(_DWORD *)(v280 + 4 * _R12) = 0;
        *(_DWORD *)(_R9 + 4 * _R12) = 0;
        v75 = v279;
        v216 = v279 == 0;
        if ( v279 > 0 )
          goto LABEL_79;
LABEL_77:
        __asm { vucomiss xmm6, dword ptr [r13+1F4h] }
        if ( !v216 )
        {
          *(_DWORD *)(_R13 + 500) = 0;
          *(_DWORD *)(_R11 + 4 * _R12) = 0;
          *(_DWORD *)(_R9 + 4 * _R12) = 0;
        }
        goto LABEL_79;
      }
      v75 = v279;
      _RCX = v280;
      __asm
      {
        vmovss  xmm1, cs:dword_125CD78
        vmovss  xmm12, cs:dword_125CDA4
        vcvtsi2ss xmm4, xmm7, eax
      }
      __asm { vcvtsi2ss xmm5, xmm7, eax }
      __asm { vcvtsi2ss xmm6, xmm7, eax }
      __asm
      {
        vaddss  xmm10, xmm5, xmm4
        vsubss  xmm4, xmm4, xmm5
        vcvtsi2ss xmm7, xmm7, eax
      }
      __asm
      {
        vaddss  xmm11, xmm7, xmm6
        vsubss  xmm6, xmm6, xmm7
        vcvtsi2ss xmm8, xmm8, eax
      }
      __asm
      {
        vcvtsi2ss xmm9, xmm3, eax
        vmovss  xmm0, cs:dword_125CD6C
      }
      __asm
      {
        vmulss  xmm2, xmm6, xmm1
        vmovss  xmm1, cs:dword_125CD7C
        vaddss  xmm7, xmm9, xmm8
        vsubss  xmm3, xmm8, xmm9
        vmovss  xmm8, cs:dword_125CD68
        vmovss  xmm9, cs:dword_125CDA0
        vmulss  xmm0, xmm11, xmm0
        vmulss  xmm5, xmm10, xmm8
        vaddss  xmm0, xmm0, xmm5
        vmulss  xmm5, xmm7, xmm13
        vaddss  xmm0, xmm0, xmm5
        vmovss  [rbp+var_50], xmm0
        vmovss  xmm0, cs:dword_125CD74
        vmulss  xmm0, xmm4, xmm0
        vaddss  xmm0, xmm2, xmm0
        vmulss  xmm2, xmm3, xmm1
        vaddss  xmm0, xmm0, xmm2
        vmovss  [rbp+var_4C], xmm0
        vmovss  xmm0, cs:dword_125CD80
        vmulss  xmm2, xmm11, xmm0
        vmovss  xmm0, cs:dword_125CD84
        vmulss  xmm1, xmm10, xmm0
        vmovss  xmm0, cs:dword_125CD88
        vaddss  xmm1, xmm2, xmm1
        vmulss  xmm2, xmm7, xmm0
        vmovss  xmm0, cs:dword_125CD8C
        vmulss  xmm7, xmm7, xmm9
        vaddss  xmm13, xmm1, xmm2
        vmovss  xmm1, cs:dword_125CD9C
        vmulss  xmm2, xmm6, xmm0
        vmovss  xmm0, cs:dword_125CD90
        vmulss  xmm6, xmm6, xmm14
        vmulss  xmm1, xmm11, xmm1
        vmulss  xmm5, xmm4, xmm0
        vmovss  xmm0, cs:dword_125CD94
        vmulss  xmm4, xmm4, xmm12
        vmovss  xmm12, cs:dword_125CDAC
        vaddss  xmm2, xmm2, xmm5
        vaddss  xmm4, xmm6, xmm4
        vmulss  xmm5, xmm3, xmm0
        vmulss  xmm3, xmm3, xmm12
        vaddss  xmm2, xmm2, xmm5
        vcvtsi2ss xmm5, xmm14, eax
        vmovss  xmm0, cs:dword_125CD98
        vaddss  xmm3, xmm4, xmm3
        vaddss  xmm4, xmm15, cs:dword_125CD64
      }
      __asm
      {
        vmulss  xmm0, xmm10, xmm0
        vaddss  xmm0, xmm1, xmm0
        vmulss  xmm3, xmm3, xmm4
        vaddss  xmm10, xmm0, xmm7
        vcvtsi2ss xmm11, xmm14, eax
      }
      __asm { vcvtsi2ss xmm7, xmm14, eax }
      __asm
      {
        vaddss  xmm3, xmm10, xmm3
        vcvtsi2ss xmm6, xmm14, eax
      }
      __asm
      {
        vmulss  xmm3, xmm3, xmm4
        vaddss  xmm2, xmm2, xmm3
        vmovaps xmm3, xmm8
        vsubss  xmm8, xmm5, xmm11
        vcvtsi2ss xmm0, xmm14, eax
      }
      __asm
      {
        vmulss  xmm2, xmm2, xmm4
        vcvtsi2ss xmm1, xmm14, eax
        vaddss  xmm2, xmm13, xmm2
        vmovss  xmm13, cs:dword_125CD70
        vmulss  xmm2, xmm2, xmm4
        vaddss  xmm2, xmm2, [rbp+var_4C]
        vmulss  xmm2, xmm2, xmm4
        vaddss  xmm2, xmm2, [rbp+var_50]
        vmovss  dword ptr [rcx+r12*4], xmm2
        vaddss  xmm2, xmm11, xmm5
        vaddss  xmm5, xmm6, xmm7
        vsubss  xmm6, xmm7, xmm6
        vaddss  xmm7, xmm1, xmm0
        vsubss  xmm1, xmm0, xmm1
        vmulss  xmm0, xmm2, xmm3
        vmulss  xmm3, xmm5, cs:dword_125CD6C
        vaddss  xmm0, xmm3, xmm0
        vmulss  xmm3, xmm7, xmm13
        vaddss  xmm15, xmm0, xmm3
        vmulss  xmm3, xmm8, cs:dword_125CD74
        vmulss  xmm0, xmm6, cs:dword_125CD78
        vaddss  xmm0, xmm0, xmm3
        vmulss  xmm3, xmm1, cs:dword_125CD7C
        vaddss  xmm10, xmm0, xmm3
        vmulss  xmm0, xmm2, cs:dword_125CD84
        vmulss  xmm3, xmm5, cs:dword_125CD80
        vmulss  xmm2, xmm2, cs:dword_125CD98
        vaddss  xmm0, xmm3, xmm0
        vmulss  xmm3, xmm7, cs:dword_125CD88
        vaddss  xmm11, xmm0, xmm3
        vmulss  xmm3, xmm6, cs:dword_125CD8C
        vmulss  xmm0, xmm8, cs:dword_125CD90
        vaddss  xmm0, xmm3, xmm0
        vmulss  xmm3, xmm1, cs:dword_125CD94
        vmulss  xmm1, xmm1, xmm12
        vaddss  xmm0, xmm0, xmm3
        vmulss  xmm3, xmm5, cs:dword_125CD9C
        vmulss  xmm5, xmm6, xmm14
        vaddss  xmm2, xmm3, xmm2
        vmulss  xmm3, xmm7, xmm9
        vaddss  xmm2, xmm2, xmm3
        vmulss  xmm3, xmm8, cs:dword_125CDA4
        vaddss  xmm3, xmm5, xmm3
        vxorps  xmm5, xmm5, xmm5
        vaddss  xmm1, xmm3, xmm1
        vmulss  xmm1, xmm1, xmm4
        vaddss  xmm1, xmm2, xmm1
        vmovss  xmm2, cs:dword_125CDB0
        vmulss  xmm1, xmm1, xmm4
        vaddss  xmm0, xmm0, xmm1
        vmulss  xmm0, xmm0, xmm4
        vaddss  xmm0, xmm11, xmm0
        vmulss  xmm0, xmm0, xmm4
        vaddss  xmm0, xmm10, xmm0
        vmulss  xmm0, xmm0, xmm4
        vaddss  xmm0, xmm15, xmm0
        vmovss  dword ptr [r9+r12*4], xmm0
      }
      v16 = *(_DWORD *)(_R13 + 488);
      __asm
      {
        vmovss  xmm4, dword ptr [r13+1F4h]
        vmovss  xmm1, dword ptr [r13+1D4h]
      }
      v201 = *(_QWORD *)(_R13 + 280);
      __asm
      {
        vcvtsi2ss xmm0, xmm14, dword ptr [rdx+rcx+88h]
        vaddss  xmm0, xmm0, xmm4
        vdivss  xmm0, xmm0, xmm1
        vmulss  xmm0, xmm0, xmm2
        vmovss  xmm2, cs:dword_125CD60
        vmaxss  xmm0, xmm5, xmm0
        vmovss  dword ptr [r13+1E4h], xmm0
        vmulss  xmm1, xmm1, xmm2
      }
      _RDX = *(unsigned int *)(_R13 + 528);
      __asm
      {
        vmulss  xmm0, xmm1, xmm0
        vcvttss2si r11d, xmm0
      }
      v8 = (unsigned int)(_R11D - *(_DWORD *)(_R13 + 528));
      if ( _R11D - *(_DWORD *)(_R13 + 528) < 1 )
        v8 = (unsigned int)(*(_DWORD *)(_R13 + 528) - _R11D);
      if ( (int)v8 > 0 )
      {
        __asm
        {
          vmovss  xmm6, dword ptr [r13+21Ch]
          vxorps  xmm5, xmm5, xmm5
          vucomiss xmm6, xmm5
        }
        __asm
        {
          vxorps  xmm5, xmm5, xmm5
          vucomiss xmm6, xmm5
        }
        if ( (int)_RDX >= _R11D )
        {
          __asm
          {
            vmovsd  xmm0, qword ptr [r13+0E8h]
            vcvtsd2ss xmm0, xmm0, xmm0
            vdivss  xmm5, xmm6, xmm0
          }
        }
      }
      __asm
      {
        vaddss  xmm0, xmm4, xmm5
        vmovss  xmm1, cs:dword_125CDB4
        vmovss  xmm3, cs:dword_125CD5C
        vxorps  xmm6, xmm6, xmm6
        vmovss  dword ptr [r13+1F4h], xmm0
      }
      _R11 = v280;
      v216 = v279 == 0;
      if ( v279 <= 0 )
        goto LABEL_77;
    }
    else
    {
      v227 = v74 + 2;
      v228 = v74 + 3;
      if ( (unsigned __int64)v74 < v278 )
        v74 = (__int16 *)(v272 - (v278 - (_QWORD)v74));
      v229 = v74 + 1;
      if ( (unsigned __int64)v227 < v272 )
        v229 = v228;
      v230 = (__int16 *)((char *)v227 + v278 - v272 + 2);
      _R11 = v280;
      v201 = v66;
      if ( v274 >= v275 || (unsigned __int64)v227 < v272 )
        v230 = v229;
      v8 = (unsigned int)*v74;
      __asm { vcvtsi2ss xmm4, xmm7, esi }
      __asm
      {
        vcvtsi2ss xmm5, xmm7, ecx
        vmulss  xmm5, xmm5, xmm15
        vaddss  xmm4, xmm5, xmm4
        vmulss  xmm4, xmm4, xmm1
        vmovss  dword ptr [r11+r12*4], xmm4
      }
      _RDX = (unsigned int)(*v230 - v74[1]);
      __asm
      {
        vcvtsi2ss xmm4, xmm7, ecx
        vcvtsi2ss xmm5, xmm7, edx
        vmulss  xmm5, xmm5, xmm15
        vaddss  xmm4, xmm5, xmm4
        vmulss  xmm4, xmm4, xmm1
        vmovss  dword ptr [r9+r12*4], xmm4
        vmovss  xmm4, dword ptr [r13+1D8h]
        vmulss  xmm4, xmm4, dword ptr [r13+1DCh]
        vaddss  xmm4, xmm4, dword ptr [r13+1F4h]
        vmovss  dword ptr [r13+1F4h], xmm4
      }
      v75 = v279;
      v216 = v279 == 0;
      if ( v279 <= 0 )
        goto LABEL_77;
    }
LABEL_79:
    if ( v274 >= v275 )
    {
      __asm
      {
        vcvtsi2ss xmm4, xmm9, dword ptr [rbx+rcx+0A0h]
        vucomiss xmm4, dword ptr [r13+1F4h]
      }
      if ( __CFSHL__(3LL * v16, 6) )
        __asm { vmovss  dword ptr [r13+1F4h], xmm4 }
    }
    v245 = false;
    if ( v75 <= 0 )
    {
      __asm { vucomiss xmm6, dword ptr [r13+1D8h] }
      v246 = 192LL * v16;
      v245 = *(_DWORD *)(v201 + v246 + 136) != 0;
      _RDX = (unsigned int)-*(_DWORD *)(v201 + v246 + 136);
      __asm
      {
        vcvtsi2ss xmm0, xmm9, edx
        vucomiss xmm0, dword ptr [r13+1F4h]
      }
      if ( *(_DWORD *)(v201 + v246 + 136) == 0 )
      {
LABEL_95:
        v251 = 0;
        goto LABEL_96;
      }
    }
    __asm
    {
      vmovss  xmm4, dword ptr [r13+1F4h]
      vucomiss xmm3, xmm4
    }
    if ( !v245 )
    {
      v248 = -1431655765 * ((unsigned __int64)(*(_QWORD *)(_R13 + 288) - v201) >> 6);
      v16 = (v16 - 1) % v248 + (v248 & (((v16 - 1) % v248) >> 31));
      *(_DWORD *)(_R13 + 488) = v16;
      __asm
      {
        vcvtsi2ss xmm0, xmm9, dword ptr [r13+1C0h]
        vaddss  xmm4, xmm4, xmm0
      }
      goto LABEL_89;
    }
    if ( v274 < v275 )
    {
      __asm
      {
        vcvtsi2ss xmm5, xmm9, dword ptr [r13+1C0h]
        vucomiss xmm4, xmm5
      }
      if ( v274 >= (unsigned int)v275 )
      {
        __asm { vsubss  xmm4, xmm4, xmm5 }
        v16 = (v16 + 1) % (int)(-1431655765 * ((unsigned __int64)(*(_QWORD *)(_R13 + 288) - v201) >> 6));
        *(_DWORD *)(_R13 + 488) = v16;
LABEL_89:
        __asm { vmovss  dword ptr [r13+1F4h], xmm4 }
        v69 = *(_QWORD *)(v201 + 192LL * v16 + 144);
        v271 = 1;
LABEL_90:
        v249 = 192LL * v16;
        _RDX = (unsigned int)(*(_DWORD *)(v201 + v249 + 64) - 1);
        if ( (unsigned int)_RDX < 2 )
          goto LABEL_95;
        if ( (*(_DWORD *)(v201 + v249 + 164) | 0x10) != 0x10 )
        {
          a2 = v273;
          if ( *(_DWORD *)(v273 + 16) != 0 && *(_BYTE *)(v273 + 100) == 0 )
          {
            v7 = v270;
            if ( *(int *)(v273 + 104) > 0 )
            {
              v269 = 0;
              do
                memset(*(_QWORD *)(v273 + 8 * v269++ + 24), 0, 4LL * *(int *)(v273 + 96));
              while ( v269 < *(int *)(v273 + 104) );
            }
          }
          else
          {
            v256 = *(_QWORD *)(v273 + 24);
            v257 = sub_D3D30(v273 + 88, *(double *)&_XMM0);
            a2 = v273;
            memset(v256, 0, 4LL * v257);
            v7 = v270;
          }
          goto LABEL_12;
        }
        goto LABEL_92;
      }
    }
    __asm
    {
      vcvtsi2ss xmm0, xmm9, dword ptr [rbx+rcx+0A0h]
      vucomiss xmm4, xmm0
    }
    if ( !__CFSHL__(3LL * v16, 6) )
      break;
    if ( (v271 & 1) != 0 )
      goto LABEL_90;
LABEL_92:
    __asm { vroundss xmm0, xmm4, xmm4, 9 }
    _RDX = v273;
    ++_R12;
    v66 = v201;
    _R8 = (unsigned int)(_R8 + 1);
    __asm { vcvttss2si r11d, xmm0 }
    v74 = (__int16 *)(v69 + 4LL * _R11D);
    if ( _R12 >= *(int *)(v273 + 96) )
    {
      _R11 = v280;
      v251 = 0;
      LODWORD(_R12) = _R8;
      goto LABEL_96;
    }
  }
  v251 = 1;
LABEL_96:
  _RCX = *(unsigned int *)(v273 + 96);
  if ( (int)_R12 >= (int)_RCX )
    goto LABEL_115;
  v252 = (int)_RCX;
  v8 = (int)_R12;
  v253 = (int)_RCX - (__int64)(int)_R12;
  if ( v253 < 0x20 )
  {
    do
    {
LABEL_114:
      *(_DWORD *)(_R11 + 4 * v8) = 0;
      *(_DWORD *)(_R9 + 4 * v8++) = 0;
    }
    while ( v8 < v252 );
    goto LABEL_115;
  }
  _RDX = _R11;
  _R8 = v201;
  v254 = v253 & 0xFFFFFFFFFFFFFFE0LL;
  if ( (v253 & 0xFFFFFFFFFFFFFFE0LL) == 0 )
  {
    _R11 = _RDX;
    goto LABEL_114;
  }
  _RCX = _R9;
  v255 = _RDX;
  if ( _RDX + 4LL * (int)_R12 < _RCX + 4 * v252 )
  {
    _RDX = _RCX + 4LL * (int)_R12;
    if ( _RDX < v255 + 4 * v252 )
    {
      _R11 = v255;
      _R9 = _RCX;
      goto LABEL_114;
    }
  }
  v259 = v251;
  v260 = _RCX;
  _RCX += 4LL * (int)_R12 + 96;
  _RDX = v255 + 4LL * (int)_R12 + 96;
  v8 = (int)_R12 + v254;
  __asm { vxorps  ymm0, ymm0, ymm0 }
  v262 = v253 & 0xFFFFFFFFFFFFFFE0LL;
  do
  {
    __asm
    {
      vmovups ymmword ptr [rdx-60h], ymm0
      vmovups ymmword ptr [rdx-40h], ymm0
      vmovups ymmword ptr [rdx-20h], ymm0
      vmovups ymmword ptr [rdx], ymm0
      vmovups ymmword ptr [rcx-60h], ymm0
      vmovups ymmword ptr [rcx-40h], ymm0
      vmovups ymmword ptr [rcx-20h], ymm0
      vmovups ymmword ptr [rcx], ymm0
    }
    _RCX += 128;
    _RDX += 128LL;
    v262 -= 32LL;
  }
  while ( v262 != 0 );
  v18 = v253 == v254;
  _R11 = v280;
  _R9 = v260;
  v251 = v259;
  if ( !v18 )
    goto LABEL_114;
LABEL_115:
  if ( v251 != 0 )
  {
    *(_DWORD *)(_R13 + 28) = 6;
    (*(void (__fastcall **)(_QWORD, signed __int64, unsigned __int64, __int64, __int64, __int64))(**(_QWORD **)(_R13 + 312)
                                                                                                + 16LL))(
      *(_QWORD *)(_R13 + 312),
      v8,
      _RDX,
      _RCX,
      _R8,
      _R9);
  }
  __asm { vxorps  xmm1, xmm1, xmm1 }
  __asm
  {
    vcvtsi2ss xmm0, xmm9, dword ptr [rbx+rax+88h]
    vaddss  xmm0, xmm0, dword ptr [r13+1F4h]
    vdivss  xmm0, xmm0, dword ptr [r13+1D4h]
    vmulss  xmm0, xmm0, cs:dword_125CDB0
    vmaxss  xmm0, xmm1, xmm0
    vmovss  dword ptr [r13+1E4h], xmm0
  }
  if ( (v271 & 1) != 0 )
    fmod_buffered_stream_generator_refill_buffers(_R13);
  *(_BYTE *)(v273 + 124) = 0;
  v59 = v270;
  v58 = *v270;
  return (*(__int64 (__fastcall **)(_QWORD *))(v58 + 56))(v59);
}
