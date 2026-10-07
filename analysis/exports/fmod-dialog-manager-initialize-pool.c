__int64 __fastcall fmod_dialog_manager_initialize_pool(__int64 a1, __int64 a2)
{
  unsigned __int64 v3; // rbx
  unsigned __int128 v4; // rax
  __int64 v5; // rdi
  __int64 result; // rax
  __int64 v8; // r12
  bool v11; // cc
  __int64 v12; // r15
  __int64 v13; // r13
  __int64 v14; // rbx
  _QWORD *v15; // rcx

  v3 = *(int *)(a1 + 8);
  v4 = 0x1A0 * (unsigned __int128)v3;
  *(_QWORD *)&v4 = 416 * v3 + 16;
  v5 = -1;
  if ( 416 * v3 >= 0xFFFFFFFFFFFFFFF0LL )
    *(_QWORD *)&v4 = -1;
  if ( !__OFADD__(127, (0x1A0 * (unsigned __int128)v3) >> 64 != 0) )
    v5 = v4;
  result = sub_37BF60(v5, a2, *((_QWORD *)&v4 + 1));
  v8 = result + 16;
  *(_QWORD *)(result + 8) = v3;
  if ( v3 != 0 )
  {
    _R15 = result + 16;
    do
    {
      *(_QWORD *)_R15 = &unk_18DCD58;
      __asm { vxorps  xmm0, xmm0, xmm0 }
      *(_QWORD *)(_R15 + 8) = 19246190;
      *(_QWORD *)(_R15 + 16) = 0;
      *(_QWORD *)(_R15 + 24) = 0x500000000LL;
      _InterlockedExchange((volatile __int32 *)(_R15 + 32), 0);
      *(_DWORD *)(_R15 + 36) = -1;
      *(_QWORD *)(_R15 + 48) = _R15 + 40;
      *(_QWORD *)(_R15 + 40) = _R15 + 40;
      __asm { vmovups xmmword ptr [r15+38h], xmm0 }
      *(_QWORD *)(_R15 + 112) = 0;
      *(_QWORD *)_R15 = &vtable_FmodDialogGenerator;
      result = fmod_studio_sound_generator_construct(_R15 + 136, _XMM0);
      *(_QWORD *)(_R15 + 384) = 0;
      _R15 += 416;
    }
    while ( _R15 != v8 + 416 * v3 );
    v11 = *(_DWORD *)(a1 + 8) <= 0;
    *(_QWORD *)(a1 + 64) = v8;
    if ( !v11 )
    {
      v12 = a1 + 32;
      v13 = 0;
      v14 = 40;
      do
      {
        (*(void (__fastcall **)(__int64, __int64, _QWORD))(*(_QWORD *)(v8 + v14 - 40) + 152LL))(
          v8 + v14 - 40,
          a1,
          (unsigned int)v13);
        v8 = *(_QWORD *)(a1 + 64);
        ++v13;
        *(_QWORD *)(v8 + v14 + 16) = v12;
        ++*(_QWORD *)(a1 + 48);
        v15 = *(_QWORD **)(a1 + 40);
        *(_QWORD *)(v8 + v14 + 8) = v15;
        *(_QWORD *)(v8 + v14) = v12;
        *v15 = v8 + v14;
        *(_QWORD *)(a1 + 40) = v8 + v14;
        v14 += 416;
        result = *(int *)(a1 + 8);
      }
      while ( v13 < result );
    }
  }
  else
  {
    *(_QWORD *)(a1 + 64) = v8;
  }
  return result;
}
