__int64 __fastcall fmod_studio_sound_generator_update(__int64 a1)
{
  __int64 v3; // rdi
  int v4; // eax
  __int64 v6; // rdi
  _DWORD *v7; // rax
  char v8; // cf
  char v9; // zf
  bool v14; // zf
  void (__fastcall *v23)(__int64); // rax
  __int64 v24; // rdi
  void (__fastcall *v34)(__int64); // rax
  __int64 v35; // rdi
  int v41; // [rsp+Ch] [rbp-54h] BYREF
  _BYTE v42[48]; // [rsp+10h] [rbp-50h] BYREF
  __int64 v43; // [rsp+40h] [rbp-20h]

  _RBX = (_BYTE *)a1;
  v43 = 0x6365786562696C2FLL;
  v3 = *(_QWORD *)(a1 + 192);
  if ( v3 == 0 || (unsigned int)FMOD::Studio::EventInstance::getPlaybackState(v3, &v41) == 30 )
    goto LABEL_9;
  v4 = v41;
  if ( v41 == 4 )
  {
    if ( (*(unsigned __int8 (__fastcall **)(_BYTE *))(*(_QWORD *)_RBX + 144LL))(_RBX) != 0 )
    {
      FMOD::Studio::EventInstance::stop(*((_QWORD *)_RBX + 24), 1);
      goto LABEL_11;
    }
    v4 = v41;
  }
  if ( v4 == 2 )
  {
    FMOD::Studio::EventInstance::release(*((FMOD::Studio::EventInstance **)_RBX + 24));
LABEL_9:
    *((_DWORD *)_RBX + 7) = 5;
    _RBX[204] = 1;
    LODWORD(_R14) = 0;
    return (unsigned int)_R14;
  }
LABEL_11:
  v6 = *((_QWORD *)_RBX + 8);
  if ( v6 != 0 )
  {
    v7 = (_DWORD *)(*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v6 + 136LL))(v6);
    _R14 = v42;
    audio_build_fmod_3d_attributes(v7, (__int64)v42);
    FMOD::Studio::EventInstance::set3DAttributes(*((_QWORD *)_RBX + 24), v42);
  }
  *(double *)&_XMM0 = (*(double (__fastcall **)(_BYTE *))(*(_QWORD *)_RBX + 32LL))(_RBX);
  __asm
  {
    vmovaps xmm3, xmm0
    vmovss  xmm0, dword ptr [rbx+0BCh]
    vucomiss xmm3, xmm0
  }
  if ( v8 | v9 )
    goto LABEL_31;
  __asm
  {
    vmovss  xmm4, cs:dword_125CF68
    vucomiss xmm4, dword ptr [rbx+60h]
  }
  if ( v9 )
  {
    v8 = 0;
    v14 = _RBX[128] == 0;
    if ( _RBX[128] != 0 )
    {
      (*(void (__fastcall **)(_BYTE *))(*(_QWORD *)_RBX + 16LL))(_RBX);
      LOBYTE(_R14) = 1;
      return (unsigned int)_R14;
    }
LABEL_24:
    __asm { vucomiss xmm4, dword ptr [rbx+98h] }
    if ( !v14 )
      goto LABEL_25;
    goto LABEL_31;
  }
  __asm
  {
    vsubss  xmm0, xmm3, xmm0
    vaddss  xmm0, xmm0, dword ptr [rbx+64h]
    vmovss  dword ptr [rbx+64h], xmm0
    vdivss  xmm0, xmm0, dword ptr [rbx+5Ch]
    vucomiss xmm0, cs:dword_125CF68
    vmovss  dword ptr [rbx+60h], xmm0
  }
  if ( !v8 )
  {
    v8 = 0;
    v14 = _RBX[104] == 0;
    if ( _RBX[104] == 0 )
    {
      _R14 = _RBX + 112;
      *((_DWORD *)_RBX + 21) = *((_DWORD *)_RBX + 22);
      v23 = *((void (__fastcall **)(__int64))_RBX + 14);
      v24 = *((_QWORD *)_RBX + 15);
      *((_DWORD *)_RBX + 24) = 1065353216;
      v8 = 0;
      v14 = v23 == nullptr;
      if ( v23 != nullptr )
      {
        __asm { vmovss  [rbp+var_58], xmm3 }
        v23(v24);
        __asm
        {
          vmovss  xmm4, cs:dword_125CF68
          vmovss  xmm3, [rbp+var_58]
        }
      }
      __asm
      {
        vxorps  xmm0, xmm0, xmm0
        vmovups xmmword ptr [r14], xmm0
      }
    }
    goto LABEL_24;
  }
  __asm
  {
    vmovss  xmm1, dword ptr [rbx+50h]
    vmovss  xmm2, dword ptr [rbx+58h]
    vsubss  xmm2, xmm2, xmm1
    vmulss  xmm0, xmm2, xmm0
    vaddss  xmm0, xmm0, xmm1
    vmovss  dword ptr [rbx+54h], xmm0
    vucomiss xmm4, dword ptr [rbx+98h]
  }
  if ( !v9 )
  {
LABEL_25:
    __asm
    {
      vsubss  xmm0, xmm3, dword ptr [rbx+0BCh]
      vaddss  xmm0, xmm0, dword ptr [rbx+9Ch]
      vmovss  dword ptr [rbx+9Ch], xmm0
      vdivss  xmm0, xmm0, dword ptr [rbx+94h]
      vucomiss xmm0, cs:dword_125CF68
      vmovss  dword ptr [rbx+98h], xmm0
    }
    if ( v8 )
    {
      __asm
      {
        vmovss  xmm1, dword ptr [rbx+88h]
        vmovss  xmm2, dword ptr [rbx+90h]
        vsubss  xmm2, xmm2, xmm1
        vmulss  xmm0, xmm2, xmm0
        vaddss  xmm0, xmm0, xmm1
        vmovss  dword ptr [rbx+8Ch], xmm0
      }
    }
    else if ( _RBX[160] == 0 )
    {
      _R14 = _RBX + 168;
      *((_DWORD *)_RBX + 35) = *((_DWORD *)_RBX + 36);
      v34 = *((void (__fastcall **)(__int64))_RBX + 21);
      v35 = *((_QWORD *)_RBX + 22);
      *((_DWORD *)_RBX + 38) = 1065353216;
      if ( v34 != nullptr )
      {
        __asm { vmovss  [rbp+var_58], xmm3 }
        v34(v35);
        __asm { vmovss  xmm3, [rbp+var_58] }
      }
      __asm
      {
        vxorps  xmm0, xmm0, xmm0
        vmovups xmmword ptr [r14], xmm0
      }
    }
  }
LABEL_31:
  __asm { vmovss  dword ptr [rbx+0BCh], xmm3 }
  LOBYTE(_R14) = 1;
  if ( *((_DWORD *)_RBX + 7) == 3 && *((_QWORD *)_RBX + 24) != 0 )
  {
    *(double *)&_XMM0 = (*(double (__fastcall **)(_BYTE *))(*(_QWORD *)_RBX + 104LL))(_RBX);
    __asm { vmulss  xmm0, xmm0, dword ptr [rbx+8Ch] }
    FMOD::Studio::EventInstance::setVolume(*((FMOD::Studio::EventInstance **)_RBX + 24), *(float *)&_XMM0);
  }
  return (unsigned int)_R14;
}
