char __fastcall fmod_studio_sound_generator_initialize_event(
        __int64 *a1,
        __int64 a2,
        __int64 a3,
        __int64 (__fastcall *a4)(int, FMOD::Studio::EventInstance *),
        __int64 *a5)
{
  __int64 v9; // rax
  int v10; // ecx
  unsigned int v22; // r13d
  __int64 v23; // rbx
  double v24; // xmm0_8
  char v26; // al
  _QWORD *v30; // rax
  _QWORD *v32; // r13
  int v34; // eax
  __int64 v35; // rdi
  _DWORD *v36; // rax
  _QWORD *v37; // [rsp+8h] [rbp-78h]
  int v39; // [rsp+14h] [rbp-6Ch] BYREF
  FMOD::Studio::EventDescription *v40; // [rsp+18h] [rbp-68h] BYREF
  _BYTE v41[48]; // [rsp+20h] [rbp-60h] BYREF
  __int64 v42; // [rsp+50h] [rbp-30h]

  _R12 = a1;
  _R14 = a3;
  v42 = 0x6365786562696C2FLL;
  v9 = a1[9];
  v10 = *(_DWORD *)(v9 + 16);
  if ( (unsigned int)(v10 - 1) <= 1 )
  {
    if ( v10 == 2 )
      a1 = *(__int64 **)(v9 + 760);
    else
      a1 = *(__int64 **)(v9 + 280);
  }
  if ( (unsigned int)FMOD::Studio::System::getEvent(a1, a2, &v40) != 0 )
    return 0;
  v39 = 0;
  *(double *)&_XMM0 = FMOD::Studio::EventDescription::getLength(v40, &v39);
  __asm { vcvtsi2ss xmm0, xmm0, [rbp+var_6C] }
  __asm { vmovss  dword ptr [r12+0C8h], xmm0 }
  v37 = _R12 + 24;
  FMOD::Studio::EventDescription::createInstance(v40, _R12 + 24);
  if ( a5 == nullptr )
    a5 = _R12;
  *(double *)&_XMM0 = FMOD::Studio::EventInstance::setUserData((FMOD::Studio::EventInstance *)_R12[24], a5);
  *((_DWORD *)_R12 + 47) = 0;
  *((_BYTE *)_R12 + 104) = 1;
  __asm { vxorps  xmm0, xmm0, xmm0 }
  *((_DWORD *)_R12 + 20) = *((_DWORD *)_R12 + 21);
  *((_DWORD *)_R12 + 22) = 1065353216;
  *((_DWORD *)_R12 + 24) = 0;
  *((_DWORD *)_R12 + 25) = 0;
  *((_BYTE *)_R12 + 104) = 0;
  __asm { vmovups xmmword ptr [r12+70h], xmm0 }
  if ( *((_BYTE *)_R12 + 104) == 0 )
  {
    _RAX = _R12 + 14;
    *((_DWORD *)_R12 + 21) = 1065353216;
    *((_DWORD *)_R12 + 24) = 1065353216;
    __asm { vmovups xmmword ptr [rax], xmm0 }
  }
  *((_BYTE *)_R12 + 128) = 0;
  if ( *(_BYTE *)(_R14 + 20) != 0 )
  {
    __asm
    {
      vmovss  xmm1, dword ptr [r14+1Ch]
      vmovss  xmm0, dword ptr [r14+18h]
      vmovss  [rbp+var_70], xmm1
      vmulss  xmm1, xmm0, cs:dword_125CF5C
      vmovss  xmm0, cs:dword_125CF60
    }
    v22 = *(_DWORD *)(_R14 + 32);
    v23 = *_R12;
    v24 = powf(*(double *)&_XMM0, *(double *)&_XMM1);
    __asm { vmovss  xmm1, [rbp+var_70] }
    (*(void (__fastcall **)(__int64 *, _QWORD, double, double))(v23 + 96))(_R12, v22, v24, *(double *)&_XMM1);
  }
  v26 = *(_BYTE *)(_R14 + 17);
  *((_BYTE *)_R12 + 160) = 1;
  *((_DWORD *)_R12 + 34) = *((_DWORD *)_R12 + 35);
  if ( v26 != 0 )
    __asm { vxorps  xmm0, xmm0, xmm0 }
  else
    __asm { vmovss  xmm0, cs:dword_125CF64 }
  __asm
  {
    vxorps  xmm1, xmm1, xmm1
    vmovss  dword ptr [r12+90h], xmm0
  }
  *((_DWORD *)_R12 + 38) = 0;
  *((_DWORD *)_R12 + 39) = 0;
  *((_BYTE *)_R12 + 160) = 0;
  __asm { vmovups xmmword ptr [r12+0A8h], xmm1 }
  *((_BYTE *)_R12 + 184) = v26;
  if ( *((_BYTE *)_R12 + 160) == 0 )
  {
    _RAX = _R12 + 21;
    __asm { vmovss  dword ptr [r12+8Ch], xmm0 }
    *((_DWORD *)_R12 + 38) = 1065353216;
    __asm { vmovups xmmword ptr [rax], xmm1 }
  }
  v30 = *(_QWORD **)(_R14 + 88);
  if ( v30 != nullptr )
  {
    _RBX = (_QWORD *)*v30;
    v32 = (_QWORD *)v30[1];
    if ( (_QWORD *)*v30 != v32 )
    {
      do
      {
        __asm { vmovss  xmm0, dword ptr [rbx+8] }
        (*(void (__fastcall **)(__int64 *, _QWORD, double))(*_R12 + 80))(_R12, *_RBX, *(double *)&_XMM0);
        _RBX += 2;
      }
      while ( _RBX != v32 );
    }
  }
  if ( a4 == nullptr )
    a4 = fmod_studio_sound_generator_event_callback;
  FMOD::Studio::EventInstance::setCallback(*v37, a4, 0xFFFFFFFFLL);
  if ( *(_BYTE *)(_R14 + 16) != 0 )
  {
    FMOD::Studio::EventInstance::setPaused((FMOD::Studio::EventInstance *)_R12[24], true);
    *((_BYTE *)_R12 + 204) = 0;
    FMOD::Studio::EventInstance::start(_R12[24]);
    v34 = 4;
  }
  else
  {
    v35 = _R12[8];
    if ( v35 != 0 )
    {
      v36 = (_DWORD *)(*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v35 + 136LL))(v35);
      audio_build_fmod_3d_attributes(v36, (__int64)v41);
      FMOD::Studio::EventInstance::set3DAttributes(*v37, v41);
    }
    *((_BYTE *)_R12 + 204) = 0;
    FMOD::Studio::EventInstance::start(_R12[24]);
    v34 = 3;
  }
  *((_DWORD *)_R12 + 7) = v34;
  return 1;
}
