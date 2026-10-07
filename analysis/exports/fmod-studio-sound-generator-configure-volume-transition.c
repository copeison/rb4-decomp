__int64 __fastcall fmod_studio_sound_generator_configure_volume_transition(
        __int64 a1,
        char a2,
        char a3,
        double a4,
        __m128 _XMM1)
{
  char v5; // zf
  _BYTE *v12; // r12
  double (__fastcall *v14)(__int64); // rax
  __int64 v15; // rdi
  __int64 result; // rax

  _RAX = &dword_124D444;
  __asm { vxorps  xmm1, xmm1, xmm1 }
  _RBX = a1;
  *(_BYTE *)(a1 + 184) = a2;
  __asm
  {
    vmovss  xmm0, dword ptr [rax]
    vucomiss xmm1, xmm0
  }
  if ( v5 )
  {
    v12 = (_BYTE *)(a1 + 160);
    if ( *(_BYTE *)(a1 + 160) == 0 )
    {
      _R13 = a1 + 168;
      *(_DWORD *)(a1 + 140) = *(_DWORD *)(a1 + 144);
      v14 = *(double (__fastcall **)(__int64))(a1 + 168);
      v15 = *(_QWORD *)(a1 + 176);
      *(_DWORD *)(_RBX + 152) = 1065353216;
      if ( v14 != nullptr )
        *(double *)&_XMM0 = v14(v15);
      __asm
      {
        vxorps  xmm0, xmm0, xmm0
        vmovups xmmword ptr [r13+0], xmm0
      }
    }
  }
  else
  {
    v12 = (_BYTE *)(a1 + 160);
    __asm { vmovss  dword ptr [rbx+94h], xmm0 }
  }
  *v12 = 1;
  __asm { vxorps  xmm0, xmm0, xmm0 }
  result = *(unsigned int *)(_RBX + 140);
  *(_DWORD *)(_RBX + 136) = result;
  if ( a2 == 0 )
    __asm { vmovss  xmm0, cs:dword_125CF70 }
  __asm
  {
    vxorps  xmm1, xmm1, xmm1
    vmovss  dword ptr [rbx+90h], xmm0
  }
  *(_DWORD *)(_RBX + 152) = 0;
  *(_DWORD *)(_RBX + 156) = 0;
  *v12 = 0;
  __asm { vmovups xmmword ptr [rbx+0A8h], xmm1 }
  if ( a3 != 0 && *v12 == 0 )
  {
    __asm { vmovss  dword ptr [rbx+8Ch], xmm0 }
    result = _RBX + 168;
    __asm { vxorps  xmm0, xmm0, xmm0 }
    *(_DWORD *)(_RBX + 152) = 1065353216;
    __asm { vmovups xmmword ptr [rax], xmm0 }
  }
  if ( *(_DWORD *)(_RBX + 28) != 3 && *(_QWORD *)(_RBX + 192) != 0 )
  {
    *(double *)&_XMM0 = (*(double (__fastcall **)(__int64))(*(_QWORD *)_RBX + 104LL))(_RBX);
    __asm { vmulss  xmm0, xmm0, dword ptr [rbx+8Ch] }
    return FMOD::Studio::EventInstance::setVolume(*(FMOD::Studio::EventInstance **)(_RBX + 192), *(float *)&_XMM0);
  }
  return result;
}
