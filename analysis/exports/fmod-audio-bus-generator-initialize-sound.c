__int64 __fastcall fmod_audio_bus_generator_initialize_sound(
        __int64 *a1,
        __int64 *a2,
        __int64 a3,
        __m128 _XMM0,
        __m128 _XMM1)
{
  __int64 *v8; // r15
  __int64 v9; // rax
  int v10; // ecx
  int v12; // eax
  int v13; // eax
  unsigned __int64 v14; // rax
  unsigned int v19; // r13d
  __int64 v20; // r12
  double v21; // xmm0_8
  char v25; // al
  __int64 v29; // r12
  __int64 v30; // rax
  void *v31; // rdx
  __int64 v32; // rax
  unsigned int v33; // r15d
  __int64 v34; // rax
  FMOD::Studio::Bus **v35; // rbx
  FMOD::Studio::Bus *v36; // rdi
  _QWORD *v38; // rbx
  void (__fastcall *v39)(_QWORD *, __int64, double); // r12
  double v40; // xmm0_8
  __int64 *v43; // [rsp+10h] [rbp-158h]
  __int64 v44; // [rsp+18h] [rbp-150h] BYREF
  __m256 v45; // [rsp+20h] [rbp-148h] BYREF
  __int64 v53; // [rsp+120h] [rbp-48h]

  _RBX = a1;
  _R14 = a3;
  v8 = a2;
  v53 = 0x6365786562696C2FLL;
  v9 = a1[9];
  v10 = *(_DWORD *)(v9 + 16);
  if ( (unsigned int)(v10 - 1) <= 1 )
  {
    if ( v10 == 2 )
    {
      if ( *(_QWORD *)(v9 + 760) != 0 )
        a2 = *(__int64 **)(v9 + 768);
    }
    else if ( *(_QWORD *)(v9 + 280) != 0 )
    {
      a2 = *(__int64 **)(v9 + 288);
    }
  }
  __asm { vxorps  xmm0, xmm0, xmm0 }
  *((_DWORD *)a1 + 7) = 0;
  __asm { vmovups xmmword ptr [rbx+50h], xmm0 }
  a1[12] = 0;
  *((_DWORD *)a1 + 26) = 10;
  a1[17] = 0;
  *((_DWORD *)a1 + 36) = 0;
  v12 = *((_DWORD *)a1 + 32);
  if ( v12 > 0 )
  {
    v13 = v12 - 1;
    *((_DWORD *)a1 + 32) = v13;
    if ( v13 == 0 )
    {
      v14 = __rdtsc();
      a1[15] += ((unsigned int)v14 | ((unsigned __int64)HIDWORD(v14) << 32)) - a1[14];
    }
  }
  a1[15] = 0;
  *((_DWORD *)a1 + 32) = 0;
  *((_DWORD *)a1 + 65) = -1082130432;
  *((_DWORD *)a1 + 66) = -1082130432;
  *((_BYTE *)a1 + 268) = 0;
  *((_DWORD *)a1 + 70) = 1065353216;
  *((_BYTE *)a1 + 284) = 0;
  *((_DWORD *)a1 + 69) = -1082130432;
  *((_DWORD *)a1 + 63) = 0;
  v43 = a2;
  if ( *(_BYTE *)(a3 + 20) != 0 )
  {
    __asm
    {
      vmovss  xmm1, dword ptr [r14+1Ch]
      vmovss  xmm0, dword ptr [r14+18h]
      vmovss  [rsp+168h+var_15C], xmm1
      vmulss  xmm1, xmm0, cs:dword_125CCE4
      vmovss  xmm0, cs:dword_125CCE8
    }
    v19 = *(_DWORD *)(a3 + 32);
    v20 = *a1;
    v21 = powf(*(double *)&_XMM0, *(double *)&_XMM1);
    __asm { vmovss  xmm1, [rsp+168h+var_15C] }
    (*(void (__fastcall **)(__int64 *, _QWORD, double, double))(v20 + 96))(_RBX, v19, v21, *(double *)&_XMM1);
  }
  else
  {
    *((_BYTE *)a1 + 176) = 1;
    __asm { vxorps  xmm0, xmm0, xmm0 }
    *((_DWORD *)a1 + 38) = *((_DWORD *)a1 + 39);
    *((_DWORD *)a1 + 40) = 1065353216;
    a1[21] = 0;
    *((_BYTE *)a1 + 176) = 0;
    __asm { vmovups xmmword ptr [rbx+0B8h], xmm0 }
    if ( *((_BYTE *)a1 + 176) == 0 )
    {
      _RAX = a1 + 23;
      *((_DWORD *)a1 + 39) = 1065353216;
      *((_DWORD *)a1 + 42) = 1065353216;
      __asm { vmovups xmmword ptr [rax], xmm0 }
    }
    *((_BYTE *)a1 + 249) = 0;
  }
  v25 = *(_BYTE *)(_R14 + 17);
  *((_BYTE *)_RBX + 224) = 1;
  *((_DWORD *)_RBX + 50) = *((_DWORD *)_RBX + 51);
  if ( v25 != 0 )
    __asm { vxorps  xmm0, xmm0, xmm0 }
  else
    __asm { vmovss  xmm0, cs:dword_125CCEC }
  __asm
  {
    vxorps  xmm1, xmm1, xmm1
    vmovss  dword ptr [rbx+0D0h], xmm0
  }
  *((_DWORD *)_RBX + 54) = 0;
  *((_DWORD *)_RBX + 55) = 0;
  *((_BYTE *)_RBX + 224) = 0;
  __asm { vmovups xmmword ptr [rbx+0E8h], xmm1 }
  *((_BYTE *)_RBX + 248) = v25;
  if ( *((_BYTE *)_RBX + 224) == 0 )
  {
    _RAX = _RBX + 29;
    __asm { vmovss  dword ptr [rbx+0CCh], xmm0 }
    *((_DWORD *)_RBX + 54) = 1065353216;
    __asm { vmovups xmmword ptr [rax], xmm1 }
  }
  v29 = _RBX[8];
  if ( v29 != 0 )
  {
    v30 = sub_5C20(&g_sound_manager);
    v31 = &loc_14090;
    if ( v29 == v30 )
      v31 = &loc_14080;
  }
  else
  {
    v31 = &loc_14080;
  }
  v32 = *v8;
  v33 = 0;
  if ( (unsigned int)FMOD::System::createSound(v43, *(_QWORD *)(v32 + 48), v31, 0, _RBX + 10) == 0 )
  {
    LOBYTE(v33) = 1;
    *((_BYTE *)_RBX + 256) = *(_BYTE *)(_R14 + 16);
    *((_DWORD *)_RBX + 7) = 2;
    if ( *(_DWORD *)(_R14 + 40) == 2 )
    {
      v34 = _RBX[9];
      v35 = (FMOD::Studio::Bus **)(_RBX + 12);
      if ( (unsigned int)FMOD::Studio::System::getBus(*(_QWORD *)(v34 + 280), *(_QWORD *)(_R14 + 48), v35) != 0 )
      {
        *v35 = nullptr;
      }
      else
      {
        v36 = *v35;
        __asm { vxorps  ymm0, ymm0, ymm0 }
        v38 = qword_19C9088;
        v39 = *(void (__fastcall **)(_QWORD *, __int64, double))(*qword_19C9088 + 16LL);
        __asm
        {
          vmovups [rsp+168h+var_68], ymm0
          vmovups [rsp+168h+var_88], ymm0
          vmovups [rsp+168h+var_A8], ymm0
          vmovups [rsp+168h+var_C8], ymm0
          vmovups [rsp+168h+var_E8], ymm0
          vmovups [rsp+168h+var_108], ymm0
          vmovups [rsp+168h+var_128], ymm0
          vmovups [rsp+168h+var_148], ymm0
        }
        FMOD::Studio::Bus::getPath(v36, (char *)&v45, 256, nullptr);
        v40 = sub_256FD0(&v44, &v45);
        v39(v38, v44, v40);
      }
    }
  }
  return v33;
}
