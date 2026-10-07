__int64 __fastcall fmod_audio_stream_manager_initialize_pool(__int64 a1, __int64 a2)
{
  unsigned __int64 v3; // rbx
  unsigned __int128 v4; // rax
  __int64 v5; // rdi
  __int64 result; // rax
  __int64 v8; // rdx
  __int64 v9; // r8
  _QWORD *v11; // rcx
  __int64 v12; // r9
  bool v13; // cc
  __int64 v14; // r15
  __int64 v15; // r12
  __int64 v16; // rbx

  v3 = *(int *)(a1 + 8);
  v4 = 0x120 * (unsigned __int128)v3;
  *(_QWORD *)&v4 = 288 * v3 + 8;
  v5 = -1;
  if ( 288 * v3 >= 0xFFFFFFFFFFFFFFF8LL )
    *(_QWORD *)&v4 = -1;
  if ( !__OFADD__(127, (0x120 * (unsigned __int128)v3) >> 64 != 0) )
    v5 = v4;
  result = sub_37BF60(v5, a2, *((_QWORD *)&v4 + 1));
  v8 = result + 8;
  *(_QWORD *)result = v3;
  if ( v3 != 0 )
  {
    v9 = 0x500000000LL;
    __asm { vxorps  xmm0, xmm0, xmm0 }
    v11 = (_QWORD *)(v8 + 288 * v3);
    v12 = 19246190;
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
      *(_QWORD *)result = &unk_18F0418;
      __asm { vmovups xmmword ptr [rax+70h], xmm0 }
      *(_DWORD *)(result + 128) = 0;
      *(_QWORD *)(result + 184) = 0;
      *(_DWORD *)(result + 168) = 1065353216;
      *(_BYTE *)(result + 176) = 0;
      *(_DWORD *)(result + 164) = 1148846080;
      *(_QWORD *)(result + 232) = 0;
      *(_DWORD *)(result + 216) = 1065353216;
      *(_BYTE *)(result + 224) = 0;
      *(_DWORD *)(result + 212) = 1148846080;
      result += 288;
    }
    while ( (_QWORD *)result != v11 );
    v13 = *(_DWORD *)(a1 + 8) <= 0;
    *(_QWORD *)(a1 + 64) = v8;
    if ( !v13 )
    {
      v14 = a1 + 32;
      v15 = 0;
      v16 = 40;
      do
      {
        (*(void (__fastcall **)(__int64, __int64, _QWORD, _QWORD *, __int64, __int64))(*(_QWORD *)(v8 + v16 - 40) + 152LL))(
          v8 + v16 - 40,
          a1,
          (unsigned int)v15,
          v11,
          v9,
          v12);
        v8 = *(_QWORD *)(a1 + 64);
        ++v15;
        *(_QWORD *)(v8 + v16 + 16) = v14;
        ++*(_QWORD *)(a1 + 48);
        v11 = *(_QWORD **)(a1 + 40);
        *(_QWORD *)(v8 + v16 + 8) = v11;
        *(_QWORD *)(v8 + v16) = v14;
        *v11 = v8 + v16;
        *(_QWORD *)(a1 + 40) = v8 + v16;
        v16 += 288;
        result = *(int *)(a1 + 8);
      }
      while ( v15 < result );
    }
  }
  else
  {
    *(_QWORD *)(a1 + 64) = v8;
  }
  return result;
}
