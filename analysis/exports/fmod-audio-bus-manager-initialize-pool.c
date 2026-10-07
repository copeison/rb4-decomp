__int64 __fastcall fmod_audio_bus_manager_initialize_pool(__int64 a1, __int64 a2)
{
  unsigned __int64 v4; // rbx
  unsigned __int128 v5; // rax
  __int64 v6; // rdi
  __int64 result; // rax
  __m128 v8; // xmm0
  __int64 v9; // r15
  __int64 v10; // rax
  bool v12; // cc
  __int64 v13; // r12
  __int64 v14; // r13
  __int64 v15; // rbx
  _QWORD *v16; // rcx
  __int64 v17; // [rsp+0h] [rbp-30h]

  v4 = *(int *)(a1 + 8);
  v5 = 0x1C0 * (unsigned __int128)v4;
  *(_QWORD *)&v5 = 448 * v4 + 8;
  v6 = -1;
  if ( 448 * v4 >= 0xFFFFFFFFFFFFFFF8LL )
    *(_QWORD *)&v5 = -1;
  if ( !__OFADD__(127, (0x1C0 * (unsigned __int128)v4) >> 64 != 0) )
    v6 = v5;
  result = sub_37BF60(v6, a2, *((_QWORD *)&v5 + 1));
  v9 = result + 8;
  *(_QWORD *)result = v4;
  if ( v4 != 0 )
  {
    v10 = 448 * v4;
    _RBX = v9;
    v17 = v9 + v10;
    do
    {
      sub_E0490(_RBX, 0, v8);
      __asm { vxorps  ymm0, ymm0, ymm0 }
      *(_QWORD *)_RBX = &unk_18F0208;
      *(_QWORD *)(_RBX + 80) = &unk_18F0338;
      __asm
      {
        vmovups ymmword ptr [rbx+191h], ymm0
        vmovups ymmword ptr [rbx+188h], ymm0
      }
      result = (unsigned int)_InterlockedExchange((volatile __int32 *)(_RBX + 440), 0);
      *(_BYTE *)(_RBX + 444) = 0;
      _RBX += 448;
    }
    while ( _RBX != v17 );
    v12 = *(_DWORD *)(a1 + 8) <= 0;
    *(_QWORD *)(a1 + 64) = v9;
    if ( !v12 )
    {
      v13 = a1 + 32;
      v14 = 0;
      v15 = 40;
      do
      {
        (*(void (__fastcall **)(__int64, __int64, _QWORD))(*(_QWORD *)(v9 + v15 - 40) + 152LL))(
          v9 + v15 - 40,
          a1,
          (unsigned int)v14);
        v9 = *(_QWORD *)(a1 + 64);
        ++v14;
        *(_QWORD *)(v9 + v15 + 16) = v13;
        ++*(_QWORD *)(a1 + 48);
        v16 = *(_QWORD **)(a1 + 40);
        *(_QWORD *)(v9 + v15 + 8) = v16;
        *(_QWORD *)(v9 + v15) = v13;
        *v16 = v9 + v15;
        *(_QWORD *)(a1 + 40) = v9 + v15;
        v15 += 448;
        result = *(int *)(a1 + 8);
      }
      while ( v14 < result );
    }
  }
  else
  {
    *(_QWORD *)(a1 + 64) = v9;
  }
  return result;
}
