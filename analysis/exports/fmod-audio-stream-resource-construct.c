__int64 __fastcall fmod_audio_stream_resource_construct(volatile __int32 *a1)
{
  char v7[8]; // [rsp+8h] [rbp-28h] BYREF
  __int64 v8; // [rsp+10h] [rbp-20h]

  _RBX = a1;
  v8 = 0x6365786562696C2FLL;
  *(_QWORD *)a1 = &unk_18E8A18;
  *((_QWORD *)a1 + 1) = 19246190;
  *(double *)&_XMM0 = sub_1AF950(a1 + 2, &byte_125D052);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  _InterlockedExchange(_RBX + 4, 0);
  *((_WORD *)_RBX + 10) = 0;
  __asm { vmovups xmmword ptr [rbx+18h], xmm0 }
  *((_DWORD *)_RBX + 10) = 0;
  *(_QWORD *)_RBX = &vtable_FmodAudioStreamResource;
  sub_256FD0(_RBX + 12, 19246190);
  *((_DWORD *)_RBX + 18) = 0;
  *((_BYTE *)_RBX + 64) = 0;
  *((_QWORD *)_RBX + 7) = 0;
  scePthreadMutexattrInit(v7);
  scePthreadMutexattrSettype(v7, 2);
  scePthreadMutexInit(_RBX + 20, v7, "hx crit sec");
  *(double *)&_XMM0 = scePthreadMutexattrDestroy(v7);
  __asm
  {
    vxorps  xmm0, xmm0, xmm0
    vmovups xmmword ptr [rbx+58h], xmm0
  }
  return 0x6365786562696C2FLL;
}
