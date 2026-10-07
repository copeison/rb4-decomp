__int64 __fastcall fmod_studio_sound_manager_initialize_pool(__int64 a1, __int64 a2)
{
  unsigned __int64 v3; // rbx
  unsigned __int128 v4; // rax
  __int64 v5; // rdi
  __int64 result; // rax
  __int64 v8; // rcx
  bool v10; // cc
  __int64 v11; // r15
  __int64 v12; // r12
  __int64 v13; // rbx
  _QWORD *v14; // rdx

  v3 = *(int *)(a1 + 8);
  v4 = 0xD0 * (unsigned __int128)v3;
  *(_QWORD *)&v4 = 208 * v3 + 8;
  v5 = -1;
  if ( 208 * v3 >= 0xFFFFFFFFFFFFFFF8LL )
    *(_QWORD *)&v4 = -1;
  if ( !__OFADD__(127, (0xD0 * (unsigned __int128)v3) >> 64 != 0) )
    v5 = v4;
  result = sub_37BF60(v5, a2, *((_QWORD *)&v4 + 1));
  v8 = result + 8;
  *(_QWORD *)result = v3;
  if ( v3 != 0 )
  {
    __asm { vxorps  xmm0, xmm0, xmm0 }
    result += 8;
    do
    {
      *(_QWORD *)result = &unk_18DCD58;
      *(_QWORD *)(result + 8) = 19246190;
      *(_QWORD *)(result + 16) = 0;
      *(_QWORD *)(result + 24) = 0x500000000LL;
      _InterlockedExchange((volatile __int32 *)(result + 32), 0);
      *(_DWORD *)(result + 36) = -1;
      *(_QWORD *)(result + 48) = result + 40;
      *(_QWORD *)(result + 40) = result + 40;
      __asm { vmovups xmmword ptr [rax+38h], xmm0 }
      *(_QWORD *)result = &vtable_FmodDialogEventRuntime;
      *(_QWORD *)(result + 112) = 0;
      *(_DWORD *)(result + 96) = 1065353216;
      *(_BYTE *)(result + 104) = 0;
      *(_DWORD *)(result + 92) = 1148846080;
      *(_QWORD *)(result + 168) = 0;
      *(_DWORD *)(result + 152) = 1065353216;
      *(_BYTE *)(result + 160) = 0;
      *(_DWORD *)(result + 148) = 1148846080;
      *(_QWORD *)(result + 192) = 0;
      *(_DWORD *)(result + 200) = 0;
      *(_BYTE *)(result + 204) = 1;
      result += 208;
    }
    while ( result != v8 + 208 * v3 );
    v10 = *(_DWORD *)(a1 + 8) <= 0;
    *(_QWORD *)(a1 + 64) = v8;
    if ( !v10 )
    {
      v11 = a1 + 32;
      v12 = 0;
      v13 = 40;
      do
      {
        (*(void (__fastcall **)(__int64, __int64, _QWORD))(*(_QWORD *)(v8 + v13 - 40) + 152LL))(
          v8 + v13 - 40,
          a1,
          (unsigned int)v12);
        v8 = *(_QWORD *)(a1 + 64);
        ++v12;
        *(_QWORD *)(v8 + v13 + 16) = v11;
        ++*(_QWORD *)(a1 + 48);
        v14 = *(_QWORD **)(a1 + 40);
        *(_QWORD *)(v8 + v13 + 8) = v14;
        *(_QWORD *)(v8 + v13) = v11;
        *v14 = v8 + v13;
        *(_QWORD *)(a1 + 40) = v8 + v13;
        v13 += 208;
        result = *(int *)(a1 + 8);
      }
      while ( v12 < result );
    }
  }
  else
  {
    *(_QWORD *)(a1 + 64) = v8;
  }
  return result;
}
