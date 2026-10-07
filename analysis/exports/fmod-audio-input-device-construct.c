__int64 __fastcall fmod_audio_input_device_construct(__int64 a1, int a2)
{
  _BYTE v9[8]; // [rsp+8h] [rbp-28h] BYREF
  __int64 v10; // [rsp+10h] [rbp-20h]

  _RBX = a1;
  v10 = 0x6365786562696C2FLL;
  sub_27B500();
  *(_QWORD *)_RBX = &unk_18F1208;
  *(_DWORD *)(_RBX + 16600) = -1;
  *(_DWORD *)(_RBX + 16604) = a2;
  sub_256FD0(_RBX + 16608, 19246190);
  *(_DWORD *)(_RBX + 16624) = 0;
  scePthreadMutexattrInit(v9);
  scePthreadMutexattrSettype(v9, 2);
  scePthreadMutexInit(_RBX + 16632, v9, "hx crit sec");
  *(double *)&_XMM0 = scePthreadMutexattrDestroy(v9);
  __asm
  {
    vxorps  ymm0, ymm0, ymm0
    vmovups ymmword ptr [rbx+4100h], ymm0
    vxorps  xmm0, xmm0, xmm0
  }
  *(_QWORD *)(_RBX + 16672) = 0;
  *(_DWORD *)(_RBX + 16680) = 4410;
  *(_DWORD *)(_RBX + 16684) = 0;
  *(_DWORD *)(_RBX + 16688) = -1;
  *(_DWORD *)(_RBX + 16692) = 0;
  *(_DWORD *)(_RBX + 16696) = 0;
  __asm { vmovups xmmword ptr [rbx+4140h], xmm0 }
  *(_DWORD *)(_RBX + 8) = 48000;
  *(_DWORD *)(_RBX + 16616) = 1195081728;
  return 0x6365786562696C2FLL;
}
