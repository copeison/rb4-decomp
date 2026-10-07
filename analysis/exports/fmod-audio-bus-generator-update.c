__int64 __fastcall fmod_audio_bus_generator_update(__int64 a1, double a2, __m128 _XMM1, __m128 _XMM2)
{
  unsigned int v4; // r14d
  int v6; // eax
  FMOD::ChannelControl *v7; // rdi
  int isPlaying; // eax
  FMOD::ChannelControl *v9; // rdi
  char v16; // cf
  char v17; // zf
  double v19; // xmm1_8
  double v20; // xmm2_8
  bool v23; // zf
  void (__fastcall *v33)(__int64); // rax
  __int64 v34; // rdi
  void (__fastcall *v45)(__int64); // rax
  __int64 v46; // rdi
  int v50; // eax
  char v51; // al
  __int64 v52; // rdi
  _DWORD *v53; // rax
  unsigned int v55; // esi
  int v56; // ecx
  int v57; // eax
  unsigned __int64 v58; // rax
  bool v61; // [rsp+7h] [rbp-59h] BYREF
  bool v62[12]; // [rsp+8h] [rbp-58h] BYREF
  _BYTE v63[36]; // [rsp+14h] [rbp-4Ch] BYREF
  __int64 v64; // [rsp+38h] [rbp-28h]

  _RBX = a1;
  v64 = 0x6365786562696C2FLL;
  v6 = *(_DWORD *)(a1 + 28);
  if ( v6 == 5 )
    return 0;
  v7 = *(FMOD::ChannelControl **)(a1 + 88);
  if ( v7 != nullptr )
  {
    v62[0] = false;
    isPlaying = FMOD::ChannelControl::isPlaying(v7, v62);
    if ( isPlaying == 30 )
    {
      *(_QWORD *)(_RBX + 88) = 0;
    }
    else if ( isPlaying == 0 )
    {
      v6 = *(_DWORD *)(_RBX + 28);
      if ( v6 == 2 )
        goto LABEL_15;
      goto LABEL_11;
    }
    v6 = 6;
    *(_DWORD *)(_RBX + 28) = 6;
  }
  if ( v6 == 2 )
  {
LABEL_15:
    fmod_audio_bus_generator_try_start_sound(_RBX);
LABEL_16:
    LOBYTE(v4) = 1;
    if ( *(_QWORD *)(_RBX + 88) == 0 )
      return v4;
    if ( *(_BYTE *)(_RBX + 284) != 0 )
    {
      __asm
      {
        vmovss  xmm0, dword ptr [rbx+114h]
        vxorps  xmm1, xmm1, xmm1
        vucomiss xmm1, xmm0
      }
    }
    if ( *(_BYTE *)(_RBX + 268) != 0 )
    {
      __asm
      {
        vmovss  xmm1, dword ptr [rbx+104h]
        vmovss  xmm0, dword ptr [rbx+108h]
        vxorps  xmm2, xmm2, xmm2
        vucomiss xmm2, xmm1
      }
      __asm { vucomiss xmm0, xmm2 }
      __asm { vcvttss2si rsi, xmm0 }
      FMOD::Channel::setLoopPoints(*(FMOD::Channel **)(_RBX + 88), _RSI, 1u, *(_DWORD *)(_RBX + 272) - 1, 2u);
      FMOD::Channel::setLoopCount(*(FMOD::Channel **)(_RBX + 88), -1);
      *(_BYTE *)(_RBX + 268) = 0;
    }
    *(double *)&_XMM0 = (*(double (__fastcall **)(__int64))(*(_QWORD *)_RBX + 40LL))(_RBX);
    __asm
    {
      vmovss  xmm4, cs:dword_125CCF0
      vmovaps xmm3, xmm0
      vucomiss xmm4, dword ptr [rbx+0A8h]
    }
    if ( v17 )
    {
      v16 = 0;
      v23 = *(_BYTE *)(_RBX + 249) == 0;
      if ( *(_BYTE *)(_RBX + 249) != 0 )
      {
        (*(void (__fastcall **)(__int64, double, double, double, double))(*(_QWORD *)_RBX + 16LL))(
          _RBX,
          *(double *)&_XMM0,
          v19,
          v20,
          *(double *)&_XMM3);
        return v4;
      }
    }
    else
    {
      __asm
      {
        vsubss  xmm0, xmm3, dword ptr [rbx+0FCh]
        vaddss  xmm0, xmm0, dword ptr [rbx+0ACh]
        vmovss  dword ptr [rbx+0ACh], xmm0
        vdivss  xmm0, xmm0, dword ptr [rbx+0A4h]
        vucomiss xmm0, cs:dword_125CCF0
        vmovss  dword ptr [rbx+0A8h], xmm0
      }
      if ( v16 )
      {
        __asm
        {
          vmovss  xmm1, dword ptr [rbx+98h]
          vmovss  xmm2, dword ptr [rbx+0A0h]
          vsubss  xmm2, xmm2, xmm1
          vmulss  xmm0, xmm2, xmm0
          vaddss  xmm0, xmm0, xmm1
          vmovss  dword ptr [rbx+9Ch], xmm0
          vucomiss xmm4, dword ptr [rbx+0D8h]
        }
        if ( v17 )
          goto LABEL_38;
        goto LABEL_32;
      }
      v16 = 0;
      v23 = *(_BYTE *)(_RBX + 176) == 0;
      if ( *(_BYTE *)(_RBX + 176) == 0 )
      {
        _R15 = _RBX + 184;
        *(_DWORD *)(_RBX + 156) = *(_DWORD *)(_RBX + 160);
        v33 = *(void (__fastcall **)(__int64))(_RBX + 184);
        v34 = *(_QWORD *)(_RBX + 192);
        *(_DWORD *)(_RBX + 168) = 1065353216;
        v16 = 0;
        v23 = v33 == nullptr;
        if ( v33 != nullptr )
        {
          __asm { vmovss  [rbp+var_60], xmm3 }
          v33(v34);
          __asm
          {
            vmovss  xmm4, cs:dword_125CCF0
            vmovss  xmm3, [rbp+var_60]
          }
        }
        __asm
        {
          vxorps  xmm0, xmm0, xmm0
          vmovups xmmword ptr [r15], xmm0
        }
      }
    }
    __asm { vucomiss xmm4, dword ptr [rbx+0D8h] }
    if ( v23 )
      goto LABEL_38;
LABEL_32:
    __asm
    {
      vsubss  xmm0, xmm3, dword ptr [rbx+0FCh]
      vaddss  xmm0, xmm0, dword ptr [rbx+0DCh]
      vmovss  dword ptr [rbx+0DCh], xmm0
      vdivss  xmm0, xmm0, dword ptr [rbx+0D4h]
      vucomiss xmm0, cs:dword_125CCF0
      vmovss  dword ptr [rbx+0D8h], xmm0
    }
    if ( v16 )
    {
      __asm
      {
        vmovss  xmm1, dword ptr [rbx+0C8h]
        vmovss  xmm2, dword ptr [rbx+0D0h]
        vsubss  xmm2, xmm2, xmm1
        vmulss  xmm0, xmm2, xmm0
        vaddss  xmm0, xmm0, xmm1
        vmovss  dword ptr [rbx+0CCh], xmm0
      }
    }
    else if ( *(_BYTE *)(_RBX + 224) == 0 )
    {
      _R15 = _RBX + 232;
      *(_DWORD *)(_RBX + 204) = *(_DWORD *)(_RBX + 208);
      v45 = *(void (__fastcall **)(__int64))(_RBX + 232);
      v46 = *(_QWORD *)(_RBX + 240);
      *(_DWORD *)(_RBX + 216) = 1065353216;
      if ( v45 != nullptr )
      {
        __asm { vmovss  [rbp+var_60], xmm3 }
        v45(v46);
        __asm { vmovss  xmm3, [rbp+var_60] }
      }
      __asm
      {
        vxorps  xmm0, xmm0, xmm0
        vmovups xmmword ptr [r15], xmm0
      }
    }
LABEL_38:
    __asm { vmovss  dword ptr [rbx+0FCh], xmm3 }
    if ( *(_DWORD *)(_RBX + 28) == 3 )
    {
      *(double *)&_XMM0 = (*(double (__fastcall **)(__int64))(*(_QWORD *)_RBX + 104LL))(_RBX);
      __asm { vmulss  xmm0, xmm0, dword ptr [rbx+0CCh] }
      FMOD::ChannelControl::setVolume(*(FMOD::ChannelControl **)(_RBX + 88), *(float *)&_XMM0);
      FMOD::Channel::getPosition(*(FMOD::Channel **)(_RBX + 88), (unsigned int *)(_RBX + 140), 1u);
      if ( v50 == 0 )
      {
        v55 = *(_DWORD *)(_RBX + 144);
        if ( v55 != -1
          && *(_DWORD *)(_RBX + 140) != v55
          && (unsigned int)FMOD::Channel::setPosition(*(FMOD::Channel **)(_RBX + 88), v55, 1u) == 0 )
        {
          *(_DWORD *)(_RBX + 140) = *(_DWORD *)(_RBX + 144);
          *(_DWORD *)(_RBX + 144) = -1;
        }
      }
    }
    v61 = false;
    if ( (unsigned int)FMOD::ChannelControl::getPaused(*(FMOD::ChannelControl **)(_RBX + 88), &v61) != 0
      || (v51 = *(_BYTE *)(_RBX + 256)) == v61
      || (unsigned int)FMOD::ChannelControl::setPaused(*(FMOD::ChannelControl **)(_RBX + 88), v51) != 0 )
    {
LABEL_43:
      if ( *(_QWORD *)(_RBX + 88) == 0 )
        return v4;
      goto LABEL_44;
    }
    v56 = *(unsigned __int8 *)(_RBX + 256);
    *(_DWORD *)(_RBX + 28) = v56 + 3;
    v57 = *(_DWORD *)(_RBX + 128);
    if ( v56 != 0 )
    {
      if ( v57 <= 0 )
        goto LABEL_43;
      *(_DWORD *)(_RBX + 128) = -v57;
      v58 = __rdtsc();
      *(_QWORD *)(_RBX + 120) += ((unsigned int)v58 | ((unsigned __int64)HIDWORD(v58) << 32)) - *(_QWORD *)(_RBX + 112);
      if ( *(_QWORD *)(_RBX + 88) == 0 )
        return v4;
    }
    else
    {
      if ( v57 >= 0 )
        goto LABEL_43;
      *(_DWORD *)(_RBX + 128) = -v57;
      *(_QWORD *)(_RBX + 112) = __rdtsc();
      if ( *(_QWORD *)(_RBX + 88) == 0 )
        return v4;
    }
LABEL_44:
    v52 = *(_QWORD *)(_RBX + 64);
    if ( v52 != 0 )
    {
      v53 = (_DWORD *)(*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v52 + 136LL))(v52);
      audio_build_fmod_3d_attributes(v53, (__int64)v62);
      FMOD::ChannelControl::set3DAttributes(*(_QWORD *)(_RBX + 88), v62, v63, 0);
    }
    return v4;
  }
LABEL_11:
  if ( v6 != 6 )
    goto LABEL_16;
  v9 = *(FMOD::ChannelControl **)(_RBX + 88);
  if ( v9 != nullptr )
  {
    FMOD::ChannelControl::stop(v9);
    *(_QWORD *)(_RBX + 88) = 0;
  }
  *(_DWORD *)(_RBX + 28) = 5;
  return 0;
}
