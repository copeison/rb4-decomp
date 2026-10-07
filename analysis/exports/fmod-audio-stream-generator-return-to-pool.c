__int64 __fastcall fmod_audio_stream_generator_return_to_pool(__int64 a1)
{
  __int64 v3; // rbx
  __int64 v4; // rax
  _QWORD *v5; // rdx
  FMOD::Studio::Bus *v6; // rdi
  _QWORD *v8; // r14
  void (__fastcall *v9)(_QWORD *, __int64, double); // rbx
  double v10; // xmm0_8
  __int64 v12; // [rsp+18h] [rbp-150h] BYREF
  __m256 v13; // [rsp+20h] [rbp-148h] BYREF
  __int64 v21; // [rsp+120h] [rbp-48h]

  v21 = 0x6365786562696C2FLL;
  v3 = *(_QWORD *)(a1 + 16);
  scePthreadMutexLock(v3 + 24);
  ++*(_DWORD *)(v3 + 16);
  *(_BYTE *)(a1 + 39) &= ~0x80u;
  *(_QWORD *)(a1 + 64) = 0;
  v4 = v3 + 32;
  if ( *(_QWORD *)(a1 + 56) != v3 + 32 )
  {
    *(_QWORD *)(a1 + 56) = v4;
    ++*(_QWORD *)(v3 + 48);
    v5 = *(_QWORD **)(v3 + 40);
    *(_QWORD *)(a1 + 48) = v5;
    *(_QWORD *)(a1 + 40) = v4;
    *v5 = a1 + 40;
    *(_QWORD *)(v3 + 40) = a1 + 40;
  }
  --*(_DWORD *)(v3 + 16);
  scePthreadMutexUnlock(v3 + 24);
  v6 = *(FMOD::Studio::Bus **)(a1 + 96);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  v8 = qword_19C9088;
  v9 = *(void (__fastcall **)(_QWORD *, __int64, double))(*qword_19C9088 + 24LL);
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
  FMOD::Studio::Bus::getPath(v6, (char *)&v13, 256, nullptr);
  v10 = sub_256FD0(&v12, &v13);
  v9(v8, v12, v10);
  *(_QWORD *)(a1 + 96) = 0;
  return 0x6365786562696C2FLL;
}
