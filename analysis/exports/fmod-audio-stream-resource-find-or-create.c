__int64 *__fastcall fmod_audio_stream_resource_find_or_create(__int64 *a1, __int64 a2)
{
  __int64 v4; // rbx
  __int64 v11; // [rsp+0h] [rbp-40h] BYREF
  _QWORD v12[7]; // [rsp+8h] [rbp-38h] BYREF

  v12[1] = 0x6365786562696C2FLL;
  if ( a2 == 19246190 )
  {
    *a1 = 0;
  }
  else
  {
    if ( byte_19C7490[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_19C7490) != 0 )
    {
      unk_19C7488 = 19246190;
      _cxa_guard_release(byte_19C7490);
    }
    if ( unk_19C7488 == 19246190 )
    {
      sub_256FD0(v12, "Resource");
      unk_19C7488 = v12[0];
    }
    sub_1AB8E0(&v11, a2);
    v4 = v11;
    if ( v11 != 0 )
    {
      sub_1ADEB0(v11);
      if ( v11 != 0 )
        sub_1ADEF0(v11);
      (*(void (__fastcall **)(__int64))(*(_QWORD *)v4 + 8LL))(v4);
      if ( byte_19F2A18[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_19F2A18) != 0 )
      {
        unk_19F2A10 = 19246190;
        _cxa_guard_release(byte_19F2A18);
      }
      if ( unk_19F2A10 == 19246190 )
      {
        sub_256FD0(v12, "FmodAudioStreamResource");
        unk_19F2A10 = v12[0];
      }
      *a1 = v4;
      sub_1ADEB0(v4);
      sub_1ADEF0(v4);
    }
    else
    {
      _RBX = sub_37BF40(104);
      *(_QWORD *)_RBX = &unk_18E8A18;
      *(_QWORD *)(_RBX + 8) = 19246190;
      *(double *)&_XMM0 = sub_1AF950(_RBX + 8, &byte_125D052);
      __asm { vxorps  xmm0, xmm0, xmm0 }
      _InterlockedExchange((volatile __int32 *)(_RBX + 16), 0);
      *(_WORD *)(_RBX + 20) = 0;
      __asm { vmovups xmmword ptr [rbx+18h], xmm0 }
      *(_DWORD *)(_RBX + 40) = 0;
      *(_QWORD *)_RBX = &vtable_FmodAudioStreamResource;
      sub_256FD0(_RBX + 48, 19246190);
      *(_DWORD *)(_RBX + 72) = 0;
      *(_BYTE *)(_RBX + 64) = 0;
      *(_QWORD *)(_RBX + 56) = 0;
      scePthreadMutexattrInit(v12);
      scePthreadMutexattrSettype(v12, 2);
      scePthreadMutexInit(_RBX + 80, v12, "hx crit sec");
      *(double *)&_XMM0 = scePthreadMutexattrDestroy(v12);
      __asm { vxorps  xmm0, xmm0, xmm0 }
      __asm { vmovups xmmword ptr [rbx+58h], xmm0 }
      sub_1ACD40(_RBX, a2, 1);
      (*(void (__fastcall **)(__int64))(*(_QWORD *)_RBX + 24LL))(_RBX);
      *a1 = _RBX;
      sub_1ADEB0(_RBX);
    }
  }
  return a1;
}
