char __fastcall fmod_streaming_clip_create_bus_generator(__int64 a1, __int64 a2)
{
  __int64 v4; // rbx
  __int64 v5; // r13
  __int64 v6; // r14
  int v7; // eax
  __int64 v8; // rcx
  __int64 *v9; // rax
  __int64 *v10; // r13
  __int64 v11; // rcx
  double v12; // xmm0_8
  __int64 v14; // [rsp+10h] [rbp-40h]
  _QWORD v15[7]; // [rsp+18h] [rbp-38h] BYREF

  v15[1] = 0x6365786562696C2FLL;
  if ( byte_19F2CB0[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_19F2CB0) != 0 )
  {
    unk_19F2CA8 = 19246190;
    _cxa_guard_release(byte_19F2CB0);
  }
  if ( unk_19F2CA8 == 19246190 )
  {
    sub_256FD0(v15, "FmodAudioBusGeneratorManager");
    unk_19F2CA8 = v15[0];
  }
  v4 = sub_8090(&g_sound_manager);
  if ( v4 == 0 )
    return 0;
  v5 = *(_QWORD *)(a1 + 72);
  v14 = *(_QWORD *)(a2 + 8);
  v6 = unk_19C90E8;
  scePthreadMutexLock(v4 + 24);
  v7 = *(_DWORD *)(v4 + 16) + 1;
  *(_DWORD *)(v4 + 16) = v7;
  v8 = *(_QWORD *)(v4 + 48);
  if ( v8 != 0 )
  {
    v9 = *(__int64 **)(v4 + 32);
    if ( v5 != 0 )
      v6 = v5;
    v9[2] = 0;
    *(_QWORD *)(v4 + 48) = v8 - 1;
    v10 = v9 - 5;
    v11 = *v9;
    *(_QWORD *)(v11 + 8) = v9[1];
    *(_QWORD *)v9[1] = v11;
    *v9 = (__int64)v9;
    v9[1] = (__int64)v9;
    v9[3] = v14;
    v9[4] = v6;
    sub_40720((__int64)(v9 - 5));
    v7 = *(_DWORD *)(v4 + 16);
  }
  else
  {
    v10 = nullptr;
  }
  *(_DWORD *)(v4 + 16) = v7 - 1;
  v12 = scePthreadMutexUnlock(v4 + 24);
  *(_QWORD *)(a1 + 312) = v10;
  if ( v10 == nullptr )
    return 0;
  (*(void (__fastcall **)(__int64 *, __int64, __int64, _QWORD, double))(*v10 + 256))(v10, a1 + 80, a2, 0, v12);
  return 1;
}
